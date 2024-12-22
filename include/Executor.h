//
// Created by zhaoj on 2024/11/12.
//

#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <iostream>
#include "Comands.h"
#include "DatabaseManaager.h"
#include "Condition.h"
#include <map>

class Executor {
public:
    Executor(std::shared_ptr<DatabaseManager> dbManager, std::ostream& outputStream = std::cout)
        : dbManager(std::move(dbManager)), out(outputStream), firstSelectQuery(true) {}

    void execute(Command* cmd) {
        if (!cmd) {
            std::cerr << "No command to execute.\n";
            return;
        }

        auto type = cmd->getType();
        if (type == "CREATE_DATABASE") {
            auto c = dynamic_cast<CreateDatabaseCommand*>(cmd);
            if (!c) return;
            handleCreateDatabase(c);
        } else if (type == "USE_DATABASE") {
            auto c = dynamic_cast<UseDatabaseCommand*>(cmd);
            if (!c) return;
            handleUseDatabase(c);
        } else if (type == "CREATE_TABLE") {
            auto c = dynamic_cast<CreateTableCommand*>(cmd);
            if (!c) return;
            handleCreateTable(c);
        } else if (type == "DROP_TABLE") {
            auto c = dynamic_cast<DropTableCommand*>(cmd);
            if (!c) return;
            handleDropTable(c);
        } else if (type == "INSERT") {
            auto c = dynamic_cast<InsertCommand*>(cmd);
            if (!c) return;
            handleInsert(c);
        } else if (type == "SELECT") {
            auto c = dynamic_cast<SelectCommand*>(cmd);
            if (!c) return;
            handleSelect(c);
        } else if (type == "UPDATE") {
            auto c = dynamic_cast<UpdateCommand*>(cmd);
            if (!c) return;
            handleUpdate(c);
        } else if (type == "DELETE") {
            auto c = dynamic_cast<DeleteCommand*>(cmd);
            if (!c) return;
            handleDelete(c);
        } else {
            std::cerr << "Unknown command type: " << type << "\n";
        }
    }

private:
    std::shared_ptr<DatabaseManager> dbManager;
    std::ostream& out;
    bool firstSelectQuery;

    void handleCreateDatabase(CreateDatabaseCommand* cmd) {
        if (!dbManager->createDatabase(cmd->getDatabaseName())) {
            std::cerr << "Failed to create database: " << cmd->getDatabaseName() << "\n";
        } else {
            std::cout << "Database " << cmd->getDatabaseName() << " created.\n";
        }
    }

    void handleUseDatabase(UseDatabaseCommand* cmd) {
        if (!dbManager->useDatabase(cmd->getDatabaseName())) {
            std::cerr << "Failed to use database: " << cmd->getDatabaseName() << "\n";
        } else {
            std::cout << "Using database: " << cmd->getDatabaseName() << "\n";
        }
    }

    void handleCreateTable(CreateTableCommand* cmd) {
        auto db = dbManager->currentDB();
        if (!db) {
            std::cerr << "No database selected.\n";
            return;
        }
        if (!db->addTable(cmd->getTableName(), cmd->getColumns())) {
            std::cerr << "Failed to create table: " << cmd->getTableName() << "\n";
        } else {
            db->saveToFile();
            std::cout << "Table " << cmd->getTableName() << " created.\n";
        }
    }

    void handleDropTable(DropTableCommand* cmd) {
        auto db = dbManager->currentDB();
        if (!db) {
            std::cerr << "No database selected.\n";
            return;
        }
        if (!db->dropTable(cmd->getTableName())) {
            std::cerr << "Failed to drop table: " << cmd->getTableName() << "\n";
        } else {
            db->saveToFile();
            std::cout << "Table " << cmd->getTableName() << " dropped.\n";
        }
    }

    void handleInsert(InsertCommand* cmd) {
        auto db = dbManager->currentDB();
        if (!db) {
            std::cerr << "No database selected.\n";
            return;
        }

        auto table = db->getTable(cmd->getTableName());
        if (!table) {
            std::cerr << "Table " << cmd->getTableName() << " not found.\n";
            return;
        }

        if (!table->addRow(cmd->getValues())) {
            std::cerr << "Failed to insert row into " << cmd->getTableName() << "\n";
        } else {
            db->saveToFile();
            std::cout << "Row inserted into " << cmd->getTableName() << ".\n";
        }
    }

    void handleSelect(SelectCommand* cmd) {
        auto db = dbManager->currentDB();
        if (!db) {
            std::cerr << "No database selected.\n";
            return;
        }

        auto mainTable = db->getTable(cmd->getTableName());
        if (!mainTable) {
            std::cerr << "Table " << cmd->getTableName() << " not found.\n";
            return;
        }

        WhereClause wc = parseWhereClause(cmd->getWhereClause());
        auto rows = filterRows(mainTable, wc);

        // Current schema is mainTable's schema
        std::vector<DataType> currentSchema = mainTable->getTypeConfig();
        std::vector<std::string> currentColNames;
        if (cmd->getJoins().empty()) {
            for (auto& c : mainTable->getColumns()) {
                currentColNames.push_back(c.getTitle()); // no prefix if simply select
            }
        } else {
            for (auto& c : mainTable->getColumns()) {
                currentColNames.push_back(mainTable->getName() + "." + c.getTitle());
            }
        }

        // For each join
        for (const auto& join : cmd->getJoins()) {
            auto jt = db->getTable(join.tableName);
            if (!jt) {
                std::cerr << "Join table " << join.tableName << " not found.\n";
                return;
            }

            // Perform the join
            rows = performJoin(rows, currentSchema, currentColNames, jt, join.condition);

            // Update schema by merging jt schema
            auto jtSchema = jt->getTypeConfig();
            for (auto& dt : jtSchema) {
                currentSchema.push_back(dt);
            }

            // Update column names by prefixing join table name
            for (auto& c : jt->getColumns()) {
                currentColNames.push_back(join.tableName + "." + c.getTitle());
            }
        }

        // After all joins applied and rows filtered, print results
        printFinalSelectResults(cmd->getColumns(), currentSchema, currentColNames, rows);
    }

    void handleUpdate(UpdateCommand* cmd) {
        auto db = dbManager->currentDB();
        if (!db) {
            std::cerr << "No database selected.\n";
            return;
        }

        auto table = db->getTable(cmd->getTableName());
        if (!table) {
            std::cerr << "Table not found: " << cmd->getTableName() << "\n";
            return;
        }

        WhereClause wc = parseWhereClause(cmd->getWhereClause());
        auto& allRows = table->getRows();

        // First, map column names to indices
        std::map<std::string, int> colMap;
        for (int i = 0; i < (int)table->getColumns().size(); i++) {
            colMap[table->getColumns()[i].getTitle()] = i;
        }

        // Identify which columns to update
        std::vector<std::pair<int, std::string>> updates;
        for (auto& u : cmd->getSetClauses()) {
            auto it = colMap.find(u.first);
            if (it == colMap.end()) {
                std::cerr << "Column " << u.first << " not found in " << cmd->getTableName() << "\n";
                return;
            }
            updates.push_back({it->second, u.second});
        }

        // For each row that matches wc, update it
        for (size_t i = 0; i < allRows.size(); i++) {
            if (evaluateWhereClause(wc, *table, allRows[i])) {
                auto vals = allRows[i].getValues();
                std::vector<std::string> rawValues;
                rawValues.reserve(vals.size());
                for (auto& v : vals) {
                    rawValues.push_back(v.getRawValue());
                }

                // Apply updates
                for (auto& up : updates) {
                    rawValues[up.first] = up.second;
                }

                if (!table->updateRowValues(i, rawValues)) {
                    std::cerr << "Failed to update row at index " << i << "\n";
                }
            }
        }

        db->saveToFile();
        std::cout << "Rows updated in " << cmd->getTableName() << ".\n";
    }

    void handleDelete(DeleteCommand* cmd) {
        auto db = dbManager->currentDB();
        if (!db) {
            std::cerr << "No database selected.\n";
            return;
        }

        auto table = db->getTable(cmd->getTableName());
        if (!table) {
            std::cerr << "Table not found: " << cmd->getTableName() << "\n";
            return;
        }

        WhereClause wc = parseWhereClause(cmd->getWhereClause());
        auto& allRows = table->getRows();

        for (int i = (int)allRows.size() - 1; i >= 0; i--) {
            if (evaluateWhereClause(wc, *table, allRows[i])) {
                if (!table->deleteRow((size_t)i)) {
                    std::cerr << "Failed to delete row at index " << i << "\n";
                }
            }
        }

        db->saveToFile();
        std::cout << "Rows deleted from " << cmd->getTableName() << ".\n";
    }

    std::vector<Row> filterRows(std::shared_ptr<Table> table, const WhereClause& wc) {
        std::vector<Row> result;
        for (auto& r : table->getRows()) {
            if (evaluateWhereClause(wc, *table, r)) {
                result.push_back(r);
            }
        }
        return result;
    }

    // performJoin: Performs an INNER JOIN between the current row set (with schema)
    // and a join table based on condition "tableA.colX = tableB.colY"
    std::vector<Row> performJoin(const std::vector<Row>& leftRows,
                                 const std::vector<DataType>& leftSchema,
                                 const std::vector<std::string>& leftColNames,
                                 std::shared_ptr<Table> rightTable,
                                 const std::string& condition) {
        // Parse condition: "A.col = B.col"
        // Find '='
        auto eqPos = condition.find('=');
        if (eqPos == std::string::npos) {
            std::cerr << "Invalid join condition: " << condition << "\n";
            return {};
        }
        std::string leftCond = trimStr(condition.substr(0, eqPos));
        std::string rightCond = trimStr(condition.substr(eqPos + 1));

        auto leftDot = leftCond.find('.');
        auto rightDot = rightCond.find('.');
        if (leftDot == std::string::npos || rightDot == std::string::npos) {
            std::cerr << "Invalid join condition format.\n";
            return {};
        }

        std::string leftTableName = leftCond.substr(0, leftDot);
        std::string leftColumnName = leftCond.substr(leftDot + 1);
        std::string rightTableName = rightCond.substr(0, rightDot);
        std::string rightColumnName = rightCond.substr(rightDot + 1);

        // Find indexes in leftColNames and rightTable
        int leftIndex = -1;

        // First, try full match: "TableName.ColumnName"
        for (int i = 0; i < (int)leftColNames.size(); i++) {
            if (leftColNames[i] == leftTableName + "." + leftColumnName) {
                leftIndex = i;
                break;
            }
        }

        // If full match not found, try exact column match (no table prefix)
        if (leftIndex == -1) {
            for (int i = 0; i < (int)leftColNames.size(); i++) {
                if (leftColNames[i] == leftColumnName) {
                    leftIndex = i;
                    break;
                }
            }
        }

        // If still not found, try matching by column name alone (ignore prefix)
        if (leftIndex == -1) {
            for (int i = 0; i < (int)leftColNames.size(); i++) {
                auto pos = leftColNames[i].find('.');
                std::string colNameOnly = (pos != std::string::npos)
                                          ? leftColNames[i].substr(pos + 1)
                                          : leftColNames[i];
                if (colNameOnly == leftColumnName) {
                    leftIndex = i;
                    break;
                }
            }
        }

        if (leftIndex == -1) {
            std::cerr << "Left join column " << leftColumnName << " not found.\n";
            return {};
        }

        auto& rightCols = rightTable->getColumns();
        int rightIndex = -1;
        for (int i = 0; i < (int)rightCols.size(); i++) {
            if (rightCols[i].getTitle() == rightColumnName) {
                rightIndex = i;
                break;
            }
        }
        if (rightIndex == -1) {
            std::cerr << "Right join column " << rightColumnName << " not found.\n";
            return {};
        }

        // Now perform nested loop join
        std::vector<Row> result;
        auto rightRows = rightTable->getRows();
        for (auto& lRow : leftRows) {
            auto lVals = lRow.getValues();
            Value lVal = lVals[leftIndex];

            for (auto& rRow : rightRows) {
                auto rVals = rRow.getValues();
                Value rVal = rVals[rightIndex];
                if (lVal == rVal) {
                    // Merge rows
                    std::vector<DataType> mergedSchema = leftSchema;
                    for (auto dt : rightTable->getTypeConfig()) {
                        mergedSchema.push_back(dt);
                    }

                    std::vector<std::string> mergedRaw;
                    for (auto& lv : lVals) mergedRaw.push_back(lv.getRawValue());
                    for (auto& rv : rVals) mergedRaw.push_back(rv.getRawValue());

                    Row mergedRow(mergedSchema, mergedRaw);
                    result.push_back(mergedRow);
                }
            }
        }

        return result;
    }

    void printFinalSelectResults(const std::vector<std::string>& cols,
                                 const std::vector<DataType>& schema,
                                 const std::vector<std::string>& colNames,
                                 const std::vector<Row>& rows) {

        bool printAll = (cols.size() == 1 && (cols[0] == "*" || cols[0] == "ALL"));
        std::vector<int> colIndexes;
        if (!printAll) {
            for (auto& c : cols) {
                int idx = -1;
                // c could be "table.col" or just "col"
                for (int i = 0; i < (int)colNames.size(); i++) {
                    if (colNames[i] == c) {
                        idx = i; break;
                    }
                    // Try matching just column name without table prefix
                    auto pos = colNames[i].find('.');
                    std::string cNameOnly = (pos != std::string::npos) ? colNames[i].substr(pos+1) : colNames[i];
                    if (cNameOnly == c) {
                        idx = i; break;
                    }
                }
                if (idx == -1) {
                    std::cerr << "Column " << c << " not found in final result.\n";
                    return;
                }
                colIndexes.push_back(idx);
            }
        }

        // Print header
        if (printAll) {
            bool firstCol = true;
            for (auto& cn : colNames) {
                if (!firstCol) out << ",";
                out << cn;
                firstCol = false;
            }
            out << "\n";
        } else {
            for (size_t i = 0; i < colIndexes.size(); i++) {
                if (i > 0) out << ",";
                out << colNames[colIndexes[i]];
            }
            out << "\n";
        }

        // Print rows
        for (auto& r : rows) {
            auto vals = r.getValues();
            if (printAll) {
                for (size_t i = 0; i < vals.size(); i++) {
                    if (i > 0) out << ",";
                    printValueCSV(vals[i]);
                }
            } else {
                for (size_t i = 0; i < colIndexes.size(); i++) {
                    if (i > 0) out << ",";
                    printValueCSV(vals[colIndexes[i]]);
                }
            }
            out << "\n";
        }
        out << "---\n";
    }

    static std::string trimStr(const std::string& s) {
        size_t start = 0;
        while (start < s.size() && std::isspace((unsigned char)s[start])) start++;
        size_t end = s.size();
        while (end > start && std::isspace((unsigned char)s[end - 1])) end--;
        return s.substr(start, end - start);
    }

    void printValueCSV(const Value& v) {
        out << v.getDisplayValue();
    }

    int findJoinedColumnIndex(const std::vector<std::string>& allCols, const std::string& fullName) {
        for (int i = 0; i < (int)allCols.size(); i++) {
            if (allCols[i] == fullName) return i;
        }
        return -1;
    }

    int findJoinedColumnIndexNoTable(const std::vector<std::string>& allCols, const std::string& colName) {
        for (int i = 0; i < (int)allCols.size(); i++) {
            auto pos = allCols[i].find('.');
            if (pos != std::string::npos) {
                std::string cName = allCols[i].substr(pos+1);
                if (cName == colName) return i;
            }
        }
        return -1;
    }
};


#endif // EXECUTOR_H
