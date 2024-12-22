//
// Created by zhaoj on 2024/11/12.
//

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include "Comands.h"
#include "Utils.h"


class Parser {
public:
    Parser() = default;

    // Parse a single command string into a Command object
    static std::unique_ptr<Command> parse(const std::string& query) {
        std::string trimmed = trim(query);
        if (trimmed.empty()) return nullptr;

        std::vector<std::string> tokens = tokenize(trimmed);
        if (tokens.empty()) {
            return nullptr;
        }

        std::vector<std::string> upperTokens = tokens;
        for (auto &t : upperTokens) {
            t = toUpper(t);
        }

        // Identify the command type
        if (upperTokens.size() >= 2 && upperTokens[0] == "CREATE" && upperTokens[1] == "DATABASE") {
            return parseCreateDatabase(tokens);
        } else if (upperTokens.size() >= 2 && upperTokens[0] == "USE" && upperTokens[1] == "DATABASE") {
            return parseUseDatabase(tokens);
        } else if (upperTokens.size() >= 3 && upperTokens[0] == "CREATE" && upperTokens[1] == "TABLE") {
            return parseCreateTable(tokens);
        } else if (upperTokens.size() >= 3 && upperTokens[0] == "DROP" && upperTokens[1] == "TABLE") {
            return parseDropTable(tokens);
        } else if (upperTokens.size() >= 4 && upperTokens[0] == "INSERT" && upperTokens[1] == "INTO") {
            return parseInsert(tokens);
        } else if (!upperTokens.empty() && upperTokens[0] == "SELECT") {
            return parseSelect(tokens, upperTokens);
        } else if (!upperTokens.empty() && upperTokens[0] == "UPDATE") {
            return parseUpdate(tokens, upperTokens);
        } else if (upperTokens.size() >= 3 && upperTokens[0] == "DELETE" && upperTokens[1] == "FROM") {
            return parseDelete(tokens, upperTokens);
        }

        // Unrecognized command
        std::cerr << "Unrecognized or unsupported command: " << query << std::endl;
        return nullptr;
    }

    // Parse multiple commands from a single string (e.g., the contents of a file)
    // Commands are assumed to be separated by semicolons, but semicolons within quotes are ignored.
    static std::vector<std::unique_ptr<Command>> parseMultiple(const std::string& content) {
        std::vector<std::unique_ptr<Command>> commands;
        std::vector<std::string> statements = splitBySemicolon(content);

        for (auto &stmt : statements) {
            auto cmd = parse(stmt);
            if (cmd) {
                commands.push_back(std::move(cmd));
            }
        }

        return commands;
    }

    // Parse multiple commands from a file
    static std::vector<std::unique_ptr<Command>> parseFile(const std::string& filename) {
        std::ifstream ifs(filename);
        if (!ifs.is_open()) {
            std::cerr << "Error opening file: " << filename << "\n";
            return {};
        }

        std::stringstream buffer;
        buffer << ifs.rdbuf();
        std::string fileContent = buffer.str();
        return parseMultiple(fileContent);
    }

private:
    // Parse functions for each command type
    static std::unique_ptr<Command> parseCreateDatabase(const std::vector<std::string>& tokens) {
        // CREATE DATABASE database_name;
        if (tokens.size() < 3) {
            std::cerr << "CREATE DATABASE command requires a database name.\n";
            return nullptr;
        }
        std::string dbName = stripSemicolon(tokens[2]);
        return std::make_unique<CreateDatabaseCommand>(dbName);
    }

    static std::unique_ptr<Command> parseUseDatabase(const std::vector<std::string>& tokens) {
        // USE DATABASE database_name;
        if (tokens.size() < 3) {
            std::cerr << "USE DATABASE command requires a database name.\n";
            return nullptr;
        }
        std::string dbName = stripSemicolon(tokens[2]);
        return std::make_unique<UseDatabaseCommand>(dbName);
    }

    static std::unique_ptr<Command> parseCreateTable(const std::vector<std::string>& tokens) {
        // CREATE TABLE table_name ( col_name type, col_name type, ... );

        if (tokens.size() < 4) {
            std::cerr << "CREATE TABLE command requires a table name and column definitions.\n";
            return nullptr;
        }

        std::string tableName = tokens[2];
        std::vector<std::pair<std::string, DataType>> columns;

        // Find the opening and closing parentheses
        int openParen = -1, closeParen = -1;
        for (size_t i = 3; i < tokens.size(); ++i) {
            if (tokens[i] == "(") {
                openParen = i;
            }
            if (tokens[i] == ")") {
                closeParen = i;
                break;
            }
        }

        if (openParen == -1 || closeParen == -1 || closeParen <= openParen) {
            std::cerr << "Malformed CREATE TABLE command: Missing or misplaced parentheses.\n";
            return nullptr;
        }

        // Extract column definitions between parentheses
        std::vector<std::string> colDefs;
        for (size_t i = openParen + 1; i < closeParen; ++i) {
            if (tokens[i] == ",") continue; // Skip commas
            colDefs.push_back(tokens[i]);
        }

        // Process column definitions as pairs: name and type
        for (size_t i = 0; i + 1 < colDefs.size(); i += 2) {
            std::string colName = colDefs[i];
            std::string typeStr = colDefs[i + 1];
            try {
                DataType dt = stringToDataType(typeStr);
                columns.emplace_back(colName, dt);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Error in CREATE TABLE: " << e.what() << std::endl;
                return nullptr;
            }
        }

        return std::make_unique<CreateTableCommand>(tableName, columns);
    }

    static std::unique_ptr<Command> parseDropTable(const std::vector<std::string>& tokens) {
        // DROP TABLE table_name;
        if (tokens.size() < 3) {
            std::cerr << "DROP TABLE command requires a table name.\n";
            return nullptr;
        }
        std::string tblName = stripSemicolon(tokens[2]);
        return std::make_unique<DropTableCommand>(tblName);
    }

    static std::unique_ptr<Command> parseInsert(const std::vector<std::string>& tokens) {
        // INSERT INTO table_name VALUES (val1, val2, ...);
        if (tokens.size() < 4) {
            std::cerr << "INSERT INTO command requires a table name and values.\n";
            return nullptr;
        }

        std::string tableName = tokens[2];
        std::vector<std::string> values;

        // Find the opening and closing parentheses after VALUES
        size_t valuesIndex = 3;
        while (valuesIndex < tokens.size() && toUpper(tokens[valuesIndex]) != "VALUES") {
            ++valuesIndex;
        }

        if (valuesIndex == tokens.size()) {
            std::cerr << "INSERT INTO command missing VALUES keyword.\n";
            return nullptr;
        }

        // Expect '(' after VALUES
        if (valuesIndex + 1 >= tokens.size() || tokens[valuesIndex + 1] != "(") {
            std::cerr << "INSERT INTO command expects '(' after VALUES.\n";
            return nullptr;
        }

        // Find closing ')'
        size_t openParen = valuesIndex + 1;
        size_t closeParen = openParen;
        while (closeParen < tokens.size() && tokens[closeParen] != ")") {
            ++closeParen;
        }

        if (closeParen == tokens.size()) {
            std::cerr << "INSERT INTO command missing closing ')'.\n";
            return nullptr;
        }

        // Extract values between parentheses
        for (size_t i = openParen + 1; i < closeParen; ++i) {
            if (tokens[i] == ",") continue; // Skip commas
            std::string val = tokens[i];
            // Remove surrounding quotes if present
            if (isQuoted(val)) {
                val = unquote(val);
            }
            values.push_back(val);
        }

        return std::make_unique<InsertCommand>(tableName, values);
    }

    static std::unique_ptr<Command> parseSelect(const std::vector<std::string>& tokens, const std::vector<std::string>& upperTokens) {
        // Similar to your current code
        std::vector<std::string> columns;
        std::string tableName;
        std::string whereClause;

        // This will store multiple joins
        std::vector<SelectCommand::JoinClause> joins;

        // Find FROM index
        size_t fromIndex = findToken(upperTokens, "FROM");
        if (fromIndex == std::string::npos || fromIndex + 1 >= tokens.size()) {
            std::cerr << "SELECT command missing FROM clause.\n";
            return nullptr;
        }

        // Columns are between SELECT and FROM
        for (size_t i = 1; i < fromIndex; ++i) {
            std::string col = stripComma(tokens[i]);
            if (!col.empty()) columns.push_back(col);
        }

        // Next token after FROM is table name
        tableName = stripSemicolon(stripComma(tokens[fromIndex + 1]));

        size_t currentIndex = fromIndex + 2;
        // Now parse multiple JOIN clauses
        while (currentIndex < tokens.size()) {
            // Look for INNER JOIN
            size_t innerIndex = findToken(upperTokens, "INNER", currentIndex);
            if (innerIndex == std::string::npos) {
                // No more INNER JOINs, break out
                break;
            }
            // Ensure next token is JOIN
            if (innerIndex + 1 >= tokens.size() || toUpper(tokens[innerIndex + 1]) != "JOIN") {
                // Malformed syntax
                std::cerr << "Expected JOIN after INNER.\n";
                return nullptr;
            }

            // Extract join table
            if (innerIndex + 2 >= tokens.size()) {
                std::cerr << "INNER JOIN syntax error: Missing join table.\n";
                return nullptr;
            }
            std::string joinTable = stripComma(tokens[innerIndex + 2]);

            // Find ON
            size_t onIndex = findToken(upperTokens, "ON", innerIndex + 3);
            if (onIndex == std::string::npos || onIndex + 1 >= tokens.size()) {
                std::cerr << "INNER JOIN syntax error: Missing ON clause.\n";
                return nullptr;
            }

            // Extract join condition until we hit another INNER/WHERE/END
            size_t conditionStart = onIndex + 1;
            size_t conditionEnd = tokens.size();

            // We will stop if we reach another INNER or WHERE
            size_t nextInner = findToken(upperTokens, "INNER", conditionStart);
            size_t whereIndex = findToken(upperTokens, "WHERE", conditionStart);

            if (whereIndex != std::string::npos && whereIndex < conditionEnd) {
                conditionEnd = whereIndex;
            }
            if (nextInner != std::string::npos && nextInner < conditionEnd) {
                conditionEnd = nextInner;
            }

            std::ostringstream oss;
            for (size_t i = conditionStart; i < conditionEnd; ++i) {
                oss << tokens[i] << " ";
            }
            std::string joinCondition = trim(oss.str());
            joinCondition = stripSemicolon(joinCondition);

            SelectCommand::JoinClause jc{"INNER JOIN", joinTable, joinCondition};
            joins.push_back(jc);

            // Move currentIndex to conditionEnd, continue searching for next join or WHERE
            currentIndex = conditionEnd;
        }

        // Check if WHERE present
        size_t whereIdx = findToken(upperTokens, "WHERE", currentIndex);
        if (whereIdx != std::string::npos && whereIdx + 1 < tokens.size()) {
            std::ostringstream oss;
            for (size_t i = whereIdx + 1; i < tokens.size(); ++i) {
                oss << tokens[i] << " ";
            }
            whereClause = trim(oss.str());
            whereClause = stripSemicolon(whereClause);
        }

        return std::make_unique<SelectCommand>(columns, tableName, whereClause, joins);
    }

    static std::unique_ptr<Command> parseUpdate(const std::vector<std::string>& tokens, const std::vector<std::string>& upperTokens) {
        // UPDATE table_name SET col1 = val1, col2 = val2 WHERE condition;
        if (tokens.size() < 4) {
            std::cerr << "UPDATE command requires at least table name and SET clause.\n";
            return nullptr;
        }

        std::string tableName = tokens[1];
        std::vector<std::pair<std::string, std::string>> updates;

        // Find SET index
        size_t setIndex = findToken(upperTokens, "SET");
        if (setIndex == std::string::npos || setIndex + 1 >= tokens.size()) {
            std::cerr << "UPDATE command missing SET clause.\n";
            return nullptr;
        }

        // Find WHERE index
        size_t whereIndex = findToken(upperTokens, "WHERE", setIndex + 1);

        // Parse set clauses between SET and WHERE (or end)
        size_t endSet = (whereIndex != std::string::npos) ? whereIndex : tokens.size();
        for (size_t i = setIndex + 1; i < endSet; ) {
            std::string colName = stripComma(tokens[i]);
            if (i + 1 >= tokens.size() || tokens[i + 1] != "=") {
                std::cerr << "Malformed SET clause in UPDATE command.\n";
                return nullptr;
            }
            if (i + 2 >= tokens.size()) {
                std::cerr << "Missing value in SET clause of UPDATE command.\n";
                return nullptr;
            }
            std::string val = tokens[i + 2];
            // Remove surrounding quotes if present
            if (isQuoted(val)) {
                val = unquote(val);
            }
            updates.emplace_back(colName, val);
            i += 3; // Move past "col = val"
            if (i < endSet && tokens[i] == ",") {
                ++i; // Skip comma
            }
        }

        // Parse WHERE clause if present
        std::string whereClause;
        if (whereIndex != std::string::npos && whereIndex + 1 < tokens.size()) {
            std::ostringstream oss;
            for (size_t i = whereIndex + 1; i < tokens.size(); ++i) {
                oss << tokens[i] << " ";
            }
            whereClause = trim(oss.str());
            whereClause = stripSemicolon(whereClause);
        }

        return std::make_unique<UpdateCommand>(tableName, updates, whereClause);
    }

    static std::unique_ptr<Command> parseDelete(const std::vector<std::string>& tokens, const std::vector<std::string>& upperTokens) {
        // DELETE FROM table_name WHERE condition;
        if (tokens.size() < 3) {
            std::cerr << "DELETE FROM command requires a table name.\n";
            return nullptr;
        }

        std::string tableName = stripSemicolon(tokens[2]);
        std::string whereClause;

        // Check for WHERE
        size_t whereIndex = findToken(upperTokens, "WHERE");
        if (whereIndex != std::string::npos && whereIndex + 1 < tokens.size()) {
            std::ostringstream oss;
            for (size_t i = whereIndex + 1; i < tokens.size(); ++i) {
                oss << tokens[i] << " ";
            }
            whereClause = trim(oss.str());
            whereClause = stripSemicolon(whereClause);
        }

        return std::make_unique<DeleteCommand>(tableName, whereClause);
    }

    // -------------------
    // Helper functions
    // -------------------
    // Tokenize the input string, preserving quoted strings as single tokens
    static std::vector<std::string> tokenize(const std::string& str) {
        std::vector<std::string> tokens;
        std::string token;
        bool inQuotes = false;
        char quoteChar = '\0';

        for (size_t i = 0; i < str.size(); ++i) {
            char c = str[i];

            if (inQuotes) {
                if (c == quoteChar) {
                    inQuotes = false;
                    token += c;
                    tokens.push_back(token);
                    token.clear();
                } else {
                    token += c;
                }
            } else {
                if (c == '"' || c == '\'') {
                    inQuotes = true;
                    quoteChar = c;
                    if (!token.empty()) {
                        tokens.push_back(token);
                        token.clear();
                    }
                    token += c;
                }
                else if (std::isspace(static_cast<unsigned char>(c))) {
                    if (!token.empty()) {
                        tokens.push_back(token);
                        token.clear();
                    }
                }
                else if (c == '(' || c == ')' || c == ',' || c == ';') {
                    if (!token.empty()) {
                        tokens.push_back(token);
                        token.clear();
                    }
                    std::string punct(1, c);
                    tokens.push_back(punct);
                }
                else {
                    token += c;
                }
            }
        }

        if (!token.empty()) {
            tokens.push_back(token);
        }

        return tokens;
    }

    // Split input string into statements based on semicolon, ignoring semicolons within quotes
    static std::vector<std::string> splitBySemicolon(const std::string& content) {
        std::vector<std::string> statements;
        std::string current;
        bool inQuotes = false;
        char quoteChar = '\0';

        for (size_t i = 0; i < content.size(); ++i) {
            char c = content[i];

            if (inQuotes) {
                if (c == quoteChar) {
                    inQuotes = false;
                }
                current += c;
            }
            else {
                if (c == '"' || c == '\'') {
                    inQuotes = true;
                    quoteChar = c;
                    current += c;
                }
                else if (c == ';') {
                    if (!current.empty()) {
                        statements.push_back(trim(current));
                        current.clear();
                    }
                }
                else {
                    current += c;
                }
            }
        }

        // Add any remaining statement
        std::string leftover = trim(current);
        if (!leftover.empty()) {
            statements.push_back(leftover);
        }

        return statements;
    }

    // Convert string to uppercase
    static std::string toUpper(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(), ::toupper);
        return out;
    }

    // Find the first occurrence of a token starting from a given index
    static size_t findToken(const std::vector<std::string>& tokens, const std::string& target, size_t start = 0) {
        for (size_t i = start; i < tokens.size(); ++i) {
            if (toUpper(tokens[i]) == target) {
                return i;
            }
        }
        return std::string::npos;
    }

    // Strip trailing semicolon from a token
    static std::string stripSemicolon(const std::string& s) {
        if (!s.empty() && s.back() == ';') {
            return s.substr(0, s.size() - 1);
        }
        return s;
    }

    // Strip trailing comma from a token
    static std::string stripComma(const std::string& s) {
        if (!s.empty() && s.back() == ',') {
            return s.substr(0, s.size() - 1);
        }
        return s;
    }

    // Trim leading and trailing whitespace
    static std::string trim(const std::string& s) {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
        size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
        return s.substr(start, end - start);
    }

    // Check if a string is quoted
    static bool isQuoted(const std::string& s) {
        return (s.size() >= 2) && ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\''));
    }

    // Remove surrounding quotes from a string
    static std::string unquote(const std::string& s) {
        if (isQuoted(s)) {
            return s.substr(1, s.size() - 2);
        }
        return s;
    }
};

#endif // PARSER_H
