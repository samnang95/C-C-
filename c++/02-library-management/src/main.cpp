#include "library/Book.hpp"
#include "library/Library.hpp"
#include "ui/ConsoleUI.hpp"

#include <iostream>
#include <sstream>
#include <string>

int main() {
    using namespace libmgr;

    Library library;
    library.load();
    std::cout << "Library Management — type 'help' for commands.\n";

    std::string line;
    while (std::cout << "library> " && std::getline(std::cin, line)) {
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
        } else if (command == "add") {
            addBook(library);
            library.save();
        } else if (command == "list") {
            const auto filter = lowercase(argument);
            if (filter.empty() || filter == "available" || filter == "borrowed") {
                listBooks(library.books(), filter);
            } else {
                std::cout << "Usage: list [available|borrowed]\n";
            }
        } else if (command == "view") {
            int id{};
            if (!parseId(argument, id)) {
                std::cout << "Usage: view <id>\n";
                continue;
            }
            const Book* book = library.find(id);
            if (book) {
                printBook(*book);
            } else {
                std::cout << "Book not found.\n";
            }
        } else if (command == "search") {
            if (argument.empty()) {
                std::cout << "Usage: search <text>\n";
            } else {
                listBooks(library.search(argument), "");
            }
        } else if (command == "borrow" || command == "return" || command == "remove") {
            int id{};
            if (!parseId(argument, id)) {
                std::cout << "Usage: " << command << " <id>\n";
                continue;
            }
            Book* book = library.find(id);
            if (!book) {
                std::cout << "Book not found.\n";
                continue;
            }
            if (command == "borrow") {
                if (book->borrowed) {
                    std::cout << "That book is already borrowed by " << book->borrower << ".\n";
                    continue;
                }
                const auto borrower = prompt("Borrower name: ");
                if (borrower.empty()) {
                    std::cout << "A borrower name is required.\n";
                    continue;
                }
                book->borrowed = true;
                book->borrower = borrower;
                library.save();
                std::cout << "Book borrowed.\n";
            } else if (command == "return") {
                if (!book->borrowed) {
                    std::cout << "That book is already available.\n";
                    continue;
                }
                book->borrowed = false;
                book->borrower.clear();
                library.save();
                std::cout << "Book returned.\n";
            } else {
                const auto confirmation = lowercase(prompt("Remove '" + book->title + "'? [y/N]: "));
                if (confirmation == "y" || confirmation == "yes") {
                    library.remove(id);
                    library.save();
                    std::cout << "Book removed.\n";
                } else {
                    std::cout << "Removal cancelled.\n";
                }
            }
        } else if (command == "quit" || command == "exit") {
            if (library.save()) {
                std::cout << "Library saved. Goodbye.\n";
                return 0;
            }
            std::cerr << "Could not save " << library.dbFile() << ".\n";
            return 1;
        } else {
            std::cout << "Unknown command. Type 'help' for commands.\n";
        }
    }

    if (!library.save()) {
        std::cerr << "Could not save " << library.dbFile() << ".\n";
        return 1;
    }
    std::cout << "\nLibrary saved. Goodbye.\n";
    return 0;
}
