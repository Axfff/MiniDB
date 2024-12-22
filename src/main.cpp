//
// Created by zhaoj on 2024/11/12.
//

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "../include/Parser.h"
#include "../include/Executor.h"
#include "../include/DatabaseManaager.h"

int main(int argc, char* argv[]) {
    if (argc == 3) {
        std::string inputFile = argv[1];
        std::string outputFile = argv[2];

        // Parse commands from input file
        std::vector<std::unique_ptr<Command>> commands = Parser::parseFile(inputFile);

        // Open output file
        std::ofstream ofs(outputFile);
        if (!ofs.is_open()) {
            std::cerr << "Failed to open output file: " << outputFile << "\n";
            return 1;
        }

        std::shared_ptr<DatabaseManager> DBM = std::make_shared<DatabaseManager>();
        // Pass &ofs to Executor so it prints results there
        Executor executor(DBM, ofs);

        for (auto& cmd : commands) {
            executor.execute(cmd.get());
        }

    }
    else if (argc == 1) {
        std::cout << "Entering interactive mode. Type SQL commands followed by a semicolon (;) to execute.\n";
        std::cout << "Type EXIT; to quit.\n";

        std::shared_ptr<DatabaseManager> DBM = std::make_shared<DatabaseManager>();
        Executor executor(DBM); // Executor prints to std::cout by default

        std::string inputBuffer;
        while (true) {
            std::cout << "> "; // prompt
            if (!std::getline(std::cin, inputBuffer)) {
                // EOF reached
                break;
            }

            // Trim whitespace
            auto trim = [](const std::string& s) {
                size_t start = 0;
                while (start < s.size() && std::isspace((unsigned char)s[start])) start++;
                size_t end = s.size();
                while (end > start && std::isspace((unsigned char)s[end - 1])) end--;
                return s.substr(start, end - start);
            };

            std::string command = trim(inputBuffer);
            if (command.empty()) {
                continue; // no command entered, just prompt again
            }

            // Check for exit command
            if (command == "EXIT;" || command == "exit;") {
                std::cout << "Exiting interactive mode.\n";
                break;
            }

            // The parser expects a full SQL command. Ensure command ends with ';'
            if (command.back() != ';') {
                std::cout << "Command must end with a semicolon (;)\n";
                continue;
            }

            // Parse this single command string
            std::unique_ptr<Command> cmd = Parser::parse(command);
            if (!cmd) {
                std::cout << "Failed to parse command.\n";
                continue;
            }

            // Execute the command
            executor.execute(cmd.get());
        }
    }
    else {
        std::cerr << "Usage: " << argv[0] << " input.sql output.csv\n";
        return 1;
    }

    return 0;
}

