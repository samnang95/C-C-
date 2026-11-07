#include "task/Task.hpp"
#include "task/TaskManager.hpp"
#include "ui/ConsoleUI.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main() {
    using namespace taskmgr;

    TaskManager manager;
    manager.load();
    std::cout << "Task Manager — type 'help' for commands.\n";

    std::string line;
    while (std::cout << "task> " && std::getline(std::cin, line)) {
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
            addTask(manager);
        } else if (command == "list") {
            const auto filter = lowercase(argument);
            if (filter.empty() || filter == "open" || filter == "done") {
                listTasks(manager.tasks(), filter);
            } else {
                std::cout << "Usage: list [open|done]\n";
            }
        } else if (command == "view" || command == "done" || command == "reopen" || command == "delete") {
            int id = 0;
            if (!parseId(argument, id)) {
                std::cout << "Please provide a valid task ID.\n";
                continue;
            }
            Task* task = manager.find(id);
            if (!task) {
                std::cout << "Task " << id << " was not found.\n";
                continue;
            }

            if (command == "view") {
                printTask(*task);
            } else if (command == "done") {
                task->completed = true;
                std::cout << "Task " << id << " completed.\n";
            } else if (command == "reopen") {
                task->completed = false;
                std::cout << "Task " << id << " reopened.\n";
            } else {
                const auto answer = lowercase(prompt("Delete task " + std::to_string(id) + " permanently? [y/N]: "));
                if (answer == "y" || answer == "yes") {
                    manager.remove(id);
                    std::cout << "Task deleted.\n";
                } else {
                    std::cout << "Deletion cancelled.\n";
                }
            }
        } else if (command == "search") {
            if (argument.empty()) {
                std::cout << "Usage: search <text>\n";
                continue;
            }
            const auto query = lowercase(argument);
            std::vector<const Task*> matches;
            for (const auto& task : manager.tasks()) {
                const auto haystack = lowercase(task.title + " " + task.description + " " + joinTags(task.tags));
                if (haystack.find(query) != std::string::npos) {
                    matches.push_back(&task);
                }
            }
            listTasks(matches, "");
        } else if (command == "quit" || command == "exit") {
            if (!manager.save()) {
                std::cerr << "Could not save " << manager.dbFile() << ".\n";
                return 1;
            }
            std::cout << "Tasks saved. Goodbye.\n";
            return 0;
        } else {
            std::cout << "Unknown command. Type 'help' for commands.\n";
        }
    }

    if (!manager.save()) {
        std::cerr << "Could not save " << manager.dbFile() << ".\n";
        return 1;
    }
    std::cout << "\nTasks saved. Goodbye.\n";
    return 0;
}
