//
// Created by zhaoj on 2024/11/14.
//

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <iostream>
#include "Database.h"
#include <vector>
#include <memory>

class DatabaseManager {
public:
    DatabaseManager() : currentDatabase(nullptr) {}

    bool useDatabase(const std::string& database_name) {
        // Check if the database is already loaded
        for (auto& db : databases) {
            if (db->getName() == database_name) {
                currentDatabase = db;
                return true;
            }
        }

        // Attempt to load the database from file
        auto db = std::make_shared<Database>(database_name);
        if (db->loadFromFile()) {
            databases.push_back(db);
            currentDatabase = db;
            return true;
        } else {
            std::cout << "Database file not found." << std::endl;
            return false;
        }
    }

    bool createDatabase(const std::string& database_name) {
        // Check if the database already exists in memory
        for (const auto& db : databases) {
            if (db->getName() == database_name) {
                std::cout << "Database already exists in memory." << std::endl;
                return false;
            }
        }

        // Check if the database file already exists
        std::string filename = "./databases/" + database_name + ".db";
        std::ifstream infile(filename);
        if (infile.good()) {
            std::cout << "Database file already exists." << std::endl;
            return false;
        }

        // Create a new database and save it to file
        auto db = std::make_shared<Database>(database_name);
        if (db->saveToFile()) {
            databases.push_back(db);
            return true;
        } else {
            std::cout << "Failed to create the database file." << std::endl;
            return false;
        }
    }

    [[nodiscard]] std::shared_ptr<Database> currentDB() const { return currentDatabase; }

private:
    std::vector<std::shared_ptr<Database>> databases;
    std::shared_ptr<Database> currentDatabase;
};

#endif // DATABASEMANAGER_H
