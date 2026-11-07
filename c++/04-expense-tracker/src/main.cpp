#include "expense/Expense.hpp"
#include "expense/ExpenseTracker.hpp"
#include "ui/ConsoleUI.hpp"

#include <iostream>
#include <sstream>
#include <string>

int main() {
    using namespace exptrack;

    ExpenseTracker tracker;
    tracker.load();
    std::cout << "Expense Tracker — type 'help' for commands.\n";

    std::string line;
    while (std::cout << "expense> " && std::getline(std::cin, line)) {
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
            addExpenseInteractive(tracker);
            tracker.save();
        } else if (command == "list") {
            if (argument.empty()) {
                listExpenses(tracker.expenses());
            } else if (isValidMonth(argument)) {
                listExpenses(tracker.filterByMonth(argument));
            } else {
                listExpenses(tracker.filterByCategory(argument));
            }
        } else if (command == "view" || command == "edit" || command == "remove" || command == "delete") {
            int id = 0;
            std::string idText = argument;
            if (idText.empty()) {
                idText = prompt("Expense ID: ");
            }
            if (!parseId(idText, id)) {
                std::cout << "Usage: " << command << " <id>\n";
                continue;
            }
            Expense* exp = tracker.find(id);
            if (!exp) {
                std::cout << "Expense not found.\n";
                continue;
            }

            if (command == "view") {
                printExpense(*exp);
            } else if (command == "edit") {
                editExpenseInteractive(tracker, id);
                tracker.save();
            } else {
                const auto confirm = lowercase(prompt("Delete '" + exp->description + "'? [y/N]: "));
                if (confirm == "y" || confirm == "yes") {
                    tracker.remove(id);
                    tracker.save();
                    std::cout << "Expense deleted.\n";
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
                listExpenses(tracker.search(argument));
            }
        } else if (command == "summary" || command == "stats") {
            if (!argument.empty() && !isValidMonth(argument)) {
                std::cout << "Usage: summary [YYYY-MM]\n";
                continue;
            }
            printSummary(tracker.computeSummary(argument), argument);
        } else if (command == "quit" || command == "exit") {
            if (tracker.save()) {
                std::cout << "Expenses saved. Goodbye.\n";
                return 0;
            }
            std::cerr << "Could not save " << tracker.dbFile() << ".\n";
            return 1;
        } else {
            std::cout << "Unknown command. Type 'help' for commands.\n";
        }
    }

    if (!tracker.save()) {
        std::cerr << "Could not save " << tracker.dbFile() << ".\n";
        return 1;
    }
    std::cout << "\nExpenses saved. Goodbye.\n";
    return 0;
}

