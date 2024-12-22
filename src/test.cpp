//
// Created by zhaoj on 2024/11/15.
//

#include <iostream>
#include <sstream>
#include <string>
#include "../include/Value.h"
#include "../include/Utils.h"
#include "../include/DatabaseManaager.h"
#include "../include/Database.h"
#include "../include/Parser.h"
#include "../include/Executor.h"

int main(int argc, char *argv[]) {
    // std::cout << argv[0] << std::endl;
    //
    // if (argc > 1) {
    //     std::cout << argv[1] << std::endl;
    // } else {
    //     std::cout << "No additional arguments provided." << std::endl;
    // }

    // Value s1 = Value(DataType::TEXT, "123");
    // Value s2 = Value(DataType::TEXT, "223");
    // Value i1 = Value(DataType::INT, "123");
    // Value i2 = Value(DataType::INT, "223");
    // Value f1 = Value(DataType::FLOAT, "123");
    //
    // std::cout << (s1 == s2) << (s1 < s2) << std::endl;
    // std::cout << (f1 < Value(DataType::FLOAT, "123.1")) << std::endl;

    // std::istringstream s("1a gwd 24tf fgwa");
    // std::string s1;
    // while (std::getline(s, s1, ' ')){
    //   std::cout << s1 << std::endl;
    // }
    // std::cout << s1 << std::endl;

    // DatabaseManager dm;
    // std::cout << dm.createDatabase("hkust");
    // std::cout << dm.createDatabase("students");
    // std::cout << dm.useDatabase("stude");
    // std::cout << dm.useDatabase("hkust");
    // std::cout << dm.createDatabase("courses");

    // Database db("hkust");
    // std::cout << db.addTable("Students", {{"id", DataType::INT}, {"name", DataType::TEXT},});
    // std::cout << db.getName();
    // std::cout << db.addTable("Students", {{"id", DataType::INT}, {"name", DataType::TEXT},});
    // std::cout << db.dropTable("abc");
    // std::cout << db.dropTable("Students");


    // DatabaseManager dm;
    // std::cout << dm.createDatabase("hkust");
    // std::cout << dm.useDatabase("hkust");
    // std::cout << dm.currentDB()->addTable("Students", {{"id", DataType::INT}, {"name", DataType::TEXT},});
    // std::cout << dm.currentDB()->getTable("Students")->addRow(std::vector<std::string>{"00000000", "SanZhang"});

    // // Create a database
    // Database db("TestDB");
    //
    // // Define table configuration
    // std::vector<std::pair<std::string, DataType>> tableConfig = {
    //     {"id", DataType::INT},
    //     {"name", DataType::TEXT},
    //     {"salary", DataType::FLOAT}
    // };
    //
    // // Add a table
    // db.addTable("Employees", tableConfig);
    //
    // // Add rows to the table
    // if (const auto& table = db.getTable("Employees"); table) {
    //     std::cout << table->addRow({"1", "Alice", "50000.00001"});
    //     std::cout << table->addRow({"2", "Bob", "60000.00"});
    // }
    //
    // // Define table configuration
    // std::vector<std::pair<std::string, DataType>> tableConfig2 = {
    //     {"name", DataType::TEXT},
    //     {"age", DataType::INT},
    //     {"GPA", DataType::FLOAT}
    // };
    // // Add a table
    // db.addTable("Students", tableConfig2);
    //
    // // Add rows to the table
    // if (const auto& table = db.getTable("Students"); table) {
    //     std::cout << table->addRow({"ZhangSan", "Alice", "4.0"});
    //     std::cout << table->addRow({"LiSi", "19", "4.2"});
    //     std::cout << table->addRow({"Wangwu", "18", "4.1"});
    // }
    //
    // // Save the database to a file
    // std::cout << db.saveToFile();
    //
    // // Create a new database and load from the file
    // Database db2("EmptyDB");
    // std::cout << db2.loadFromFile();
    //
    // // Verify that data was loaded correctly
    // auto loadedTable = db2.getTable("Employees");
    // if (loadedTable) {
    //     // Output data
    //     for (const auto& row : loadedTable->getRows()) {
    //         for (const auto& value : row.getValues()) {
    //             std::cout << value.getDisplayValue() << "\t";
    //         }
    //         std::cout << std::endl;
    //     }
    // }
    // auto loadedTable2 = db2.getTable("Students");
    // if (loadedTable2) {
    //     // Output data
    //     for (const auto& row : loadedTable2->getRows()) {
    //         for (const auto& value : row.getValues()) {
    //             std::cout << value.getDisplayValue() << "\t";
    //         }
    //         std::cout << std::endl;
    //     }
    // }

//     std::string testSQL = R"( CREATE DATABASE db_university;
//    USE DATABASE db_university;
//    CREATE TABLE student (
//        ID INTEGER,
// Name TEXT,
// GPA FLOAT );
//    INSERT INTO student VALUES (1000, ’Jay Chou’, 3.0);
//    INSERT INTO student VALUES (1001, ’Taylor Swift’, 3.2);
//    INSERT INTO student VALUES (1002, ’Bob Dylan’, 3.5);
//    SELECT ID, Name, GPA FROM student;
//    SELECT ID, Name, GPA FROM student WHERE GPA > 3.1 AND GPA < 3.3;)";
//     Parser p;
//     std::vector<std::unique_ptr<Command>> commands = p.parseMultiple(testSQL);

    std::string filename = "commands.sql"; // Your SQL commands file
    std::vector<std::unique_ptr<Command>> commands = Parser::parseFile(filename);

    std::shared_ptr<DatabaseManager> DBM = std::make_shared<DatabaseManager>();
    Executor executor(DBM); // Assume Executor is properly implemented

    for (auto& cmd : commands) {
        executor.execute(cmd.get());
    }

    return 0;
}