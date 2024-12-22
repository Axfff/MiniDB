//
// Created by zhaoj on 2024/11/12.
//

#ifndef MINIDB_H
#define MINIDB_H


#include "Utils.h"
#include "Table.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>


class Database {
public:
    explicit Database(const std::string& name): name(name), filename("./databases/" + name + ".db") {
        // Attempt to load existing database file
        if (std::ifstream infile(filename); infile.good()) {
            if (!loadFromFile()) tables.clear();
        } else {
            // File doesn't exist; initialize empty database
        }
    }

    [[nodiscard]] std::string getName() const { return name; };

    bool addTable(const std::string& tableName, const std::vector<std::pair<std::string, DataType>>& tableConfig){
        for (const auto& table: tables){
          if (table->getName() == tableName) {
            std::cout << "Table " << table->getName() << " already exists" << std::endl;
            return false;
          }
        }
        tables.emplace_back(std::make_shared<Table>(tableName, tableConfig));
        return true;
    }

    bool dropTable(const std::string& tableName) {
        auto it = std::remove_if(tables.begin(), tables.end(),
                                 [&tableName](const std::shared_ptr<Table>& table) {
                                     return table->getName() == tableName;
                                 });

        if (it != tables.end()) {
          tables.erase(it, tables.end());
          return true;
        }
        std::cout << "Error: Table \"" << tableName << "\" does not exist\n" << std::endl;
        return false;
    }

    std::shared_ptr<Table> getTable(const std::string& tableName) {
        for (auto& table: tables) {
            if (table->getName() == tableName) {
                return table;
            }
        }
        return nullptr;
    }

    [[nodiscard]] bool saveToFile() const {
        std::ofstream ofs(filename);
        if (!ofs.is_open()) {
            std::cerr << "Error opening file " << filename << " for writing" << std::endl;
            return false;
        }

        // Write database name
        ofs << "DatabaseName " << name << std::endl;

        // Write number of tables
        ofs << "NumberOfTables " << tables.size() << std::endl;

        // For each table
        for (const auto& table : tables) {
            // Write table name
            ofs << "TableName " << table->getName() << std::endl;

            // Write number of columns
            ofs << "NumberOfColumns " << table->getColumns().size() << std::endl;

            // For each column, write column name and data type
            for (const auto& column : table->getColumns()) {
                ofs << "ColumnName " << column.getTitle() << std::endl;
                ofs << "DataType " << dataTypeToString(column.getType()) << std::endl;
            }

            // Write number of rows
            ofs << "NumberOfRows " << table->getRows().size() << std::endl;

            // For each row, write values
            for (const auto& row : table->getRows()) {
                const auto& values = row.getValues();
                for (size_t i = 0; i < values.size(); ++i) {
                    const auto& value = values[i];
                    ofs << value.getRawValue();

                    if (i != values.size() - 1) {
                        ofs << "\t"; // Using tab as separator
                    }
                }
                ofs << std::endl;
            }
        }

        ofs.close();
        return true;
    }

    [[nodiscard]] bool loadFromFile() {
        std::ifstream ifs(filename);
        if (!ifs.is_open()) {
            std::cerr << "Error opening file " << filename << " for reading" << std::endl;
            return false;
        }

        tables.clear(); // Clear existing tables

        std::string line, token;
        size_t numberOfTables = 0;

        // Read database name
        if (!std::getline(ifs, line)) {
            std::cerr << "Error reading database name" << std::endl;
            return false;
        }
        std::istringstream iss(line);
        iss >> token;
        if (token != "DatabaseName") {
            std::cerr << "Expected 'DatabaseName', got '" << token << "'" << std::endl;
            return false;
        }
        iss >> name;

        // Read number of tables
        if (!std::getline(ifs, line)) {
            std::cerr << "Error reading number of tables" << std::endl;
            return false;
        }
        iss.clear();
        iss.str(line);
        iss >> token >> numberOfTables;
        if (token != "NumberOfTables") {
            std::cerr << "Expected 'NumberOfTables', got '" << token << "'" << std::endl;
            return false;
        }

        // For each table
        for (size_t tableIdx = 0; tableIdx < numberOfTables; ++tableIdx) {
            // Read table name
            if (!std::getline(ifs, line)) {
                std::cerr << "Error reading table name" << std::endl;
                return false;
            }
            iss.clear();
            iss.str(line);
            iss >> token;
            if (token != "TableName") {
                std::cerr << "Expected 'TableName', got '" << token << "'" << std::endl;
                return false;
            }
            std::string tableName;
            iss >> tableName;

            // Read number of columns
            if (!std::getline(ifs, line)) {
                std::cerr << "Error reading number of columns" << std::endl;
                return false;
            }
            iss.clear();
            iss.str(line);
            size_t numberOfColumns;
            iss >> token >> numberOfColumns;
            if (token != "NumberOfColumns") {
                std::cerr << "Expected 'NumberOfColumns', got '" << token << "'" << std::endl;
                return false;
            }

            // Read columns
            std::vector<std::pair<std::string, DataType>> tableConfig;
            for (size_t colIdx = 0; colIdx < numberOfColumns; ++colIdx) {
                // Read column name
                if (!std::getline(ifs, line)) {
                    std::cerr << "Error reading column name" << std::endl;
                    return false;
                }
                iss.clear();
                iss.str(line);
                iss >> token;
                if (token != "ColumnName") {
                    std::cerr << "Expected 'ColumnName', got '" << token << "'" << std::endl;
                    return false;
                }
                std::string columnName;
                iss >> columnName;

                // Read data type
                if (!std::getline(ifs, line)) {
                    std::cerr << "Error reading data type" << std::endl;
                    return false;
                }
                iss.clear();
                iss.str(line);
                iss >> token;
                if (token != "DataType") {
                    std::cerr << "Expected 'DataType', got '" << token << "'" << std::endl;
                    return false;
                }
                std::string dataTypeStr;
                iss >> dataTypeStr;
                DataType dataType = stringToDataType(dataTypeStr);

                tableConfig.emplace_back(columnName, dataType);
            }

            // Create the table
            std::shared_ptr<Table> table = std::make_shared<Table>(tableName, tableConfig);

            // Read number of rows
            if (!std::getline(ifs, line)) {
                std::cerr << "Error reading number of rows" << std::endl;
                return false;
            }
            iss.clear();
            iss.str(line);
            size_t numberOfRows;
            iss >> token >> numberOfRows;
            if (token != "NumberOfRows") {
                std::cerr << "Expected 'NumberOfRows', got '" << token << "'" << std::endl;
                return false;
            }

            // Read rows
            for (size_t rowIdx = 0; rowIdx < numberOfRows; ++rowIdx) {
                if (!std::getline(ifs, line)) {
                    std::cerr << "Error reading row" << std::endl;
                    return false;
                }
                std::vector<std::string> rawValues;
                std::istringstream lineStream(line);
                std::string valueStr;
                while (std::getline(lineStream, valueStr, '\t')) {
                    // Remove quotes from text values
                    if (!valueStr.empty() && valueStr.front() == '\"' && valueStr.back() == '\"') {
                        valueStr = valueStr.substr(1, valueStr.size() - 2);
                    }
                    rawValues.push_back(valueStr);
                }

                if (!table->addRow(rawValues)) {
                    std::cerr << "Error adding row to table " << tableName << std::endl;
                    return false;
                }
            }

            // Add table to database
            tables.push_back(table);
        }

        ifs.close();
        return true;
    }

private:
    std::string name;
    std::string filename;
    std::vector<std::shared_ptr<Table>> tables;
};



#endif //MINIDB_H
