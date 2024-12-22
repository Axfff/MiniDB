//
// Created by zhaoj on 2024/11/12.
//

#ifndef TABLE_H
#define TABLE_H


#include <string>
#include <vector>
#include "Row.h"
#include "Column.h"

class Table {
public:
    Table(std::string tableName, const std::vector<std::pair<std::string, DataType>>& tableConfig): name(std::move(tableName)) {
        for (const auto& config: tableConfig) {
            if (!addColumn(config)) {
                std::cerr << "error adding column " << config.first << " to table " << name << std::endl;
            }
        }
    }

    [[nodiscard]] std::string getName() const { return name; };

    [[nodiscard]] bool addRow(const Row& row) {
        if (!row.isFormatFit(typeConfig)) return false;
        rows.push_back(row);

        // add to each column
        std::vector<Value> rowValues = row.getValues();
        for (size_t columnIdx = 0; columnIdx < columns.size(); columnIdx++) {
            if (!columns[columnIdx].addValue(std::make_shared<Value>(rowValues[columnIdx]))) {
                std::cerr << "Failed to add value to column " << columns[columnIdx].getTitle() << "\n";
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] bool addRow(const std::vector<std::string>& rawValues) {
        Row newRow(typeConfig, rawValues);
        return addRow(newRow);
    }

    bool deleteRow(size_t index) {
        if (index >= rows.size()) {
            std::cerr << "Invalid row index.\n";
            return false;
        }

        // Remove the row
        rows.erase(rows.begin() + index);
        // Also remove values from columns
        for (auto &col : columns) {
            if (!col.removeValueAt(index)) {
                std::cerr << "Failed to remove column value at index " << index << "\n";
                return false;
            }
        }

        return true;
    }

    // A helper method to update a specific row (and columns) given a set of new raw values.
    bool updateRowValues(size_t index, const std::vector<std::string>& newRawValues) {
        if (index >= rows.size()) {
            std::cerr << "Invalid row index.\n";
            return false;
        }

        Row newRow(typeConfig, newRawValues);
        if (!newRow.isFormatFit(typeConfig)) {
            std::cerr << "Row format does not fit table configuration.\n";
            return false;
        }

        rows[index] = newRow; // Update the row
        // Update columns
        std::vector<Value> vals = newRow.getValues();
        for (size_t c = 0; c < columns.size(); c++) {
            if (!columns[c].updateValueAt(index, std::make_shared<Value>(vals[c]))) {
                std::cerr << "Failed to update column " << columns[c].getTitle() << " at index " << index << "\n";
                return false;
            }
        }

        return true;
    }

    std::vector<Row> findRow() {
        // Just return all rows as a placeholder.
        // Future improvement: add parameters and filter by conditions here.
        return rows;
    }

    [[nodiscard]] const std::vector<Column>& getColumns() const { return columns; }

    [[nodiscard]] const std::vector<Row>& getRows() const { return rows; }

    // Provide access to the underlying data types if needed
    [[nodiscard]] const std::vector<DataType>& getTypeConfig() const {
        return typeConfig;
    }

private:
    std::string name;
    std::vector<Row> rows;
    std::vector<Column> columns;
    std::vector<DataType> typeConfig;

    bool addColumn(const std::pair<std::string, DataType>& config) {
        for (const auto& column: columns) {
            if (column.getTitle() == config.first) {
                std::cerr << "Column " << column.getTitle() << " already exists" << std::endl;
                return false;
            }
        }
        columns.emplace_back(config.first, config.second);
        typeConfig.push_back(config.second);
        return true;
    }
};



#endif //TABLE_H
