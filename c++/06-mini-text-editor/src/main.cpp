#include "editor/TextBuffer.hpp"
#include "ui/ConsoleUI.hpp"

#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char* argv[]) {
    using namespace textedit;

    const std::string initialPath = (argc > 1) ? argv[1] : "data/notes.txt";
    TextBuffer buffer(initialPath);
    buffer.load();

    std::cout << "Mini Text Editor — editing '" << buffer.filePath()
              << "' (" << buffer.lines().size() << " lines). Type 'help' for commands.\n";

    std::string line;
    while (std::cout << "editor> " && std::getline(std::cin, line)) {
        std::istringstream input(std::string(trim(line)));
        std::string command;
        input >> command;
        command = lowercase(command);

        std::string argument;
        std::getline(input, argument);
        if (!argument.empty() && argument.front() == ' ') {
            argument.erase(0, 1);
        }

        if (command.empty()) continue;

        if (command == "help") {
            printHelp();
        } else if (command == "clear" || command == "reset" || command == "cls") {
            std::cout << "\033[2J\033[H" << std::flush;
        } else if (command == "print" || command == "list" || command == "view") {
            printBuffer(buffer);
        } else if (command == "open" || command == "load") {
            auto path = std::string(trim(argument));
            if (path.empty()) path = prompt("File path: ");
            if (path.empty()) continue;
            if (buffer.load(path)) {
                std::cout << "Loaded '" << buffer.filePath() << "' (" << buffer.lines().size() << " lines).\n";
            } else {
                std::cout << "Opened new buffer for '" << buffer.filePath() << "'.\n";
            }
        } else if (command == "save" || command == "write") {
            const auto path = trim(argument);
            if (buffer.save(path)) {
                std::cout << "Saved " << buffer.lines().size() << " line(s) to '" << buffer.filePath() << "'.\n";
            } else {
                std::cout << "Could not save to '" << buffer.filePath() << "'.\n";
            }
        } else if (command == "append" || command == "add") {
            appendInteractive(buffer, argument);
        } else if (command == "insert") {
            insertInteractive(buffer, argument);
        } else if (command == "edit") {
            editInteractive(buffer, argument);
        } else if (command == "delete" || command == "remove") {
            std::string numStr = std::string(trim(argument));
            if (numStr.empty()) numStr = prompt("Line number to delete: ");
            std::size_t lineNum = 0;
            if (!parseLineNumber(numStr, lineNum) || !buffer.deleteLine(lineNum)) {
                std::cout << "Invalid line number.\n";
            } else {
                std::cout << "Deleted line " << lineNum << ".\n";
            }
        } else if (command == "find" || command == "search") {
            std::string query = std::string(trim(argument));
            if (query.empty()) query = prompt("Search text: ");
            if (query.empty()) {
                std::cout << "Usage: find <text>\n";
            } else {
                printMatches(buffer.find(query));
            }
        } else if (command == "replace") {
            replaceInteractive(buffer);
        } else if (command == "undo") {
            if (buffer.undo()) std::cout << "Undid last change.\n";
            else std::cout << "Nothing to undo.\n";
        } else if (command == "redo") {
            if (buffer.redo()) std::cout << "Redid change.\n";
            else std::cout << "Nothing to redo.\n";
        } else if (command == "stats") {
            printStats(buffer);
        } else if (command == "new") {
            buffer.clear();
            std::cout << "Buffer cleared.\n";
        } else if (command == "quit" || command == "exit") {
            if (buffer.isModified()) {
                const auto ans = lowercase(prompt("Save changes to '" + buffer.filePath() + "' before exiting? [Y/n]: "));
                if (ans.empty() || ans == "y" || ans == "yes") {
                    if (!buffer.save()) {
                        std::cerr << "Could not save '" << buffer.filePath() << "'.\n";
                        return 1;
                    }
                    std::cout << "Saved '" << buffer.filePath() << "'. ";
                }
            }
            std::cout << "Goodbye.\n";
            return 0;
        } else {
            std::cout << "Unknown command. Type 'help' for commands.\n";
        }
    }

    if (buffer.isModified()) {
        buffer.save();
    }
    std::cout << "\nGoodbye.\n";
    return 0;
}

