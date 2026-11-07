#include "inventory/Inventory.hpp"
#include "inventory/Product.hpp"
#include "ui/ConsoleUI.hpp"

#include <iostream>
#include <sstream>
#include <string>

int main() {
    using namespace invmgr;

    Inventory inventory;
    inventory.load();
    std::cout << "Inventory Management — type 'help' for commands.\n";

    std::string line;
    while (std::cout << "inventory> " && std::getline(std::cin, line)) {
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
        } else if (command == "add") {
            addProductInteractive(inventory);
            inventory.save();
        } else if (command == "list") {
            if (argument.empty()) {
                listProducts(inventory.products());
            } else if (lowercase(argument) == "low") {
                listProducts(inventory.lowStockProducts());
            } else {
                listProducts(inventory.filterByCategory(argument));
            }
        } else if (command == "view" || command == "edit" || command == "restock" ||
                   command == "sell" || command == "remove" || command == "delete") {
            int id = 0;
            std::string idText = argument;
            if (idText.empty()) {
                idText = prompt("Product ID: ");
            }
            if (!parseId(idText, id)) {
                std::cout << "Usage: " << command << " <id>\n";
                continue;
            }
            Product* product = inventory.find(id);
            if (!product) {
                std::cout << "Product not found.\n";
                continue;
            }

            if (command == "view") {
                printProduct(*product);
            } else if (command == "edit") {
                editProductInteractive(inventory, id);
                inventory.save();
            } else if (command == "restock") {
                adjustStockInteractive(inventory, id, true);
                inventory.save();
            } else if (command == "sell") {
                adjustStockInteractive(inventory, id, false);
                inventory.save();
            } else {
                const auto confirm = lowercase(prompt("Delete '" + product->name + "'? [y/N]: "));
                if (confirm == "y" || confirm == "yes") {
                    inventory.remove(id);
                    inventory.save();
                    std::cout << "Product deleted.\n";
                } else {
                    std::cout << "Deletion cancelled.\n";
                }
            }
        } else if (command == "search") {
            if (argument.empty()) {
                argument = prompt("Search text: ");
            }
            if (argument.empty()) {
                std::cout << "Usage: search <text>\n";
            } else {
                listProducts(inventory.search(argument));
            }
        } else if (command == "summary" || command == "stats") {
            printSummary(inventory.computeSummary());
        } else if (command == "quit" || command == "exit") {
            if (inventory.save()) {
                std::cout << "Inventory saved. Goodbye.\n";
                return 0;
            }
            std::cerr << "Could not save " << inventory.dbFile() << ".\n";
            return 1;
        } else {
            std::cout << "Unknown command. Type 'help' for commands.\n";
        }
    }

    if (!inventory.save()) {
        std::cerr << "Could not save " << inventory.dbFile() << ".\n";
        return 1;
    }
    std::cout << "\nInventory saved. Goodbye.\n";
    return 0;
}

