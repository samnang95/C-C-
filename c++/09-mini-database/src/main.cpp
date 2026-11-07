#include "db/Database.hpp"
#include "db/Table.hpp"
#include "ui/ConsoleUI.hpp"

#include <iostream>
#include <sstream>
#include <string>

int main() {
    using namespace minidb;

    Database db;
    db.load();
    std::cout << "Mini Relational Database — type 'help' for commands.\n";

    std::string line;
    while (std::cout << "minidb> " && std::getline(std::cin, line)) {
        std::istringstream input(std::string(trim(line)));
        std::string command;
        input >> command;
        command = lowercase(command);

        std::string argument;
        std::getline(input, argument);
        argument = std::string(trim(argument));

        if (command.empty()) continue;

        if (command == "help") {
            printHelp();
        } else if (command == "clear" || command == "reset" || command == "cls") {
            std::cout << "\033[2J\033[H" << std::flush;
        } else if (command == "tables" || command == "list") {
            listTables(db);
        } else if (command == "create") {
            createTableInteractive(db, argument);
            db.save();
        } else if (command == "describe" || command == "schema") {
            std::string tName = argument;
            if (tName.empty()) tName = prompt("Table name: ");
            const Table* t = db.findTable(tName);
            if (t) describeTable(*t);
            else std::cout << "Table not found.\n";
        } else if (command == "drop") {
            std::string tName = argument;
            if (tName.empty()) tName = prompt("Table name to drop: ");
            if (db.dropTable(tName)) {
                db.save();
                std::cout << "Dropped table '" << tName << "'.\n";
            } else {
                std::cout << "Table not found.\n";
            }
        } else if (command == "insert" || command == "add") {
            insertRowInteractive(db, argument);
            db.save();
        } else if (command == "select" || command == "query" || command == "view") {
            selectInteractive(db, argument);
        } else if (command == "update" || command == "edit") {
            updateInteractive(db, argument);
            db.save();
        } else if (command == "delete" || command == "remove") {
            deleteInteractive(db, argument);
            db.save();
        } else if (command == "quit" || command == "exit") {
            if (db.save()) {
                std::cout << "Database saved. Goodbye.\n";
                return 0;
            }
            std::cerr << "Could not save " << db.dbFile() << ".\n";
            return 1;
        } else {
            std::cout << "Unknown command. Type 'help' for commands.\n";
        }
    }

    if (!db.save()) {
        std::cerr << "Could not save " << db.dbFile() << ".\n";
        return 1;
    }
    std::cout << "\nDatabase saved. Goodbye.\n";
    return 0;
}

