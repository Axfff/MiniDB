//
// Created by zhaoj on 2024/12/8.
//

#ifndef CONDITION_H
#define CONDITION_H

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include "Utils.h"
#include "Value.h"
#include "Table.h"

// A single condition: columnName operator value
struct Condition {
    std::string columnName;
    std::string op;    // =, <>, <, >, <=, >=
    std::string value;
};

// Pseudocode structure for an expression node
struct ExpressionNode {
    bool isLeaf;
    Condition leafCondition;
    std::string op; // "AND", "OR"
    std::unique_ptr<ExpressionNode> left;
    std::unique_ptr<ExpressionNode> right;
};

std::unique_ptr<ExpressionNode> parseConditionTriple(const std::vector<std::string>& tokens, size_t& i) {
    // Expect columnName, operator, value
    if (i + 2 >= tokens.size()) return nullptr;

    Condition cond{tokens[i], tokens[i+1], tokens[i+2]};
    i += 3;

    auto node = std::make_unique<ExpressionNode>();
    node->isLeaf = true;
    node->leafCondition = cond;
    return node;
}

// Splits a string by whitespace, keeping quotes together.
inline std::vector<std::string> splitTokens(const std::string& str) {
    std::vector<std::string> tokens;
    bool inQuotes = false;
    char quoteChar = '\0';
    std::string token;
    for (size_t i = 0; i < str.size(); i++) {
        char c = str[i];
        if (inQuotes) {
            token.push_back(c);
            if (c == quoteChar) {
                inQuotes = false;
            }
        } else {
            if (c == '"' || c == '\'') {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
                inQuotes = true;
                quoteChar = c;
                token.push_back(c);
            } else if (std::isspace((unsigned char)c)) {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
            } else {
                token.push_back(c);
            }
        }
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }

    return tokens;
}

std::unique_ptr<ExpressionNode> parseWhereExpression(const std::string& whereClauseStr) {
    auto tokens = splitTokens(whereClauseStr);
    if (tokens.empty()) return nullptr;

    // We'll parse a sequence: condition (AND/OR condition) ...
    // For a more robust solution, you'd implement a proper parser.
    // This simplistic approach:
    // condition [op condition] ...

    size_t i = 0;
    auto root = parseConditionTriple(tokens, i);
    if (!root) return nullptr;

    while (i < tokens.size()) {
        std::string op = tokens[i];
        if (op == "AND" || op == "OR") {
            i++;
            auto rightNode = parseConditionTriple(tokens, i);
            if (!rightNode) return nullptr;

            // Create a parent node
            auto parent = std::make_unique<ExpressionNode>();
            parent->isLeaf = false;
            parent->op = op;
            parent->left = std::move(root);
            parent->right = std::move(rightNode);
            root = std::move(parent);
        } else {
            // Unexpected token
            i++;
        }
    }

    return root;
}

// Modify `WhereClause` to store ExpressionNode
struct WhereClause {
    std::unique_ptr<ExpressionNode> root;
};

// Trims whitespace
inline std::string trimStr(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace((unsigned char)s[start])) start++;
    size_t end = s.size();
    while (end > start && std::isspace((unsigned char)s[end - 1])) end--;
    return s.substr(start, end - start);
}

// This is a very simple parser for conditions of the form:
// WHERE col1 = 5 AND col2 < 10 OR col3 = "John"
//
// We'll only handle basic comparisons and AND/OR.
// Steps:
// 1. Tokenize the whereClause by spaces (taking care of quotes).
// 2. Identify sequences: columnName operator value
// 3. Keep track of AND/OR tokens.
inline WhereClause parseWhereClause(const std::string& whereClauseStr) {
    WhereClause wc;
    if (whereClauseStr.empty()) return wc;
    wc.root = parseWhereExpression(whereClauseStr);
    return wc;
}

inline std::string stripTablePrefix(const std::string& colName) {
    auto dotPos = colName.find('.');
    if (dotPos != std::string::npos) {
        return colName.substr(dotPos + 1); // returns everything after "Employees."
    }
    return colName; // if no dot, return as is
}

// Evaluate a single condition against a row
// Return true if the row satisfies the condition
inline bool evaluateCondition(Condition cond, const Table& table, const Row& row) {
    // Strip prefix from cond.columnName
    std::string actualColName = stripTablePrefix(cond.columnName);

    // Find column index
    int colIdx = -1;
    const auto& cols = table.getColumns();
    for (int i = 0; i < (int)cols.size(); i++) {
        if (cols[i].getTitle() == actualColName) {
            colIdx = i;
            break;
        }
    }

    if (colIdx == -1) {
        // Column not found after stripping prefix
        return false;
    }

    Value rowVal = row.getValues()[colIdx];
    // Construct a Value of the same type from cond.value
    if (rowVal.getType() == DataType::TEXT) {
        if (!cond.value.empty() && ((cond.value.front() == '\'' && cond.value.back() == '\'') || (cond.value.front() == '"' && cond.value.back() == '"'))) {
            cond.value = cond.value.substr(1, cond.value.size() - 2);
        }
    }
    Value compVal(rowVal.getType(), cond.value);

    // std::cout << "Evaluating Condition: " << cond.columnName << " " << cond.op << " " << cond.value << std::endl;
    // std::cout << "Row Value: " << rowVal.getRawValue() << ", Condition Value: " << compVal.getRawValue() << std::endl;

    // Compare based on cond.op
    if (cond.op == "=") {
        return rowVal == compVal;
    } else if (cond.op == "<>") {
        return rowVal != compVal;
    } else if (cond.op == "<") {
        return rowVal < compVal;
    } else if (cond.op == ">") {
        return rowVal > compVal;
    } else if (cond.op == "<=") {
        return rowVal <= compVal;
    } else if (cond.op == ">=") {
        return rowVal >= compVal;
    }

    return false;
}

bool evaluateExpression(const ExpressionNode* node, const Table& table, const Row& row) {
    if (node->isLeaf) {
        return evaluateCondition(node->leafCondition, table, row);
    }

    bool leftVal = evaluateExpression(node->left.get(), table, row);
    bool rightVal = evaluateExpression(node->right.get(), table, row);

    if (node->op == "AND") return leftVal && rightVal;
    if (node->op == "OR")  return leftVal || rightVal;
    // Handle error
    return false;
}

// Evaluate all conditions in a WhereClause against a row
inline bool evaluateWhereClause(const WhereClause& wc, const Table& table, const Row& row) {
    if (!wc.root) {
        // No expression means no condition => all rows pass
        return true;
    }
    return evaluateExpression(wc.root.get(), table, row);
}

#endif //CONDITION_H
