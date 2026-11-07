#include "grade/GradeBook.hpp"
#include "grade/Student.hpp"
#include "ui/ConsoleUI.hpp"

#include <iostream>
#include <sstream>
#include <string>

int main() {
    using namespace gradesys;

    GradeBook gradeBook;
    gradeBook.load();
    std::cout << "Student Grade System — type 'help' for commands.\n";

    std::string line;
    while (std::cout << "grades> " && std::getline(std::cin, line)) {
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
            addStudentInteractive(gradeBook);
            gradeBook.save();
        } else if (command == "list") {
            const auto mode = lowercase(argument);
            if (mode.empty()) {
                listStudents(gradeBook.students());
            } else if (mode == "top" || mode == "rank") {
                listStudents(gradeBook.rankedStudents());
            } else {
                std::cout << "Usage: list [top]\n";
            }
        } else if (command == "view" || command == "grade" || command == "ungrade" || command == "remove") {
            int id = 0;
            if (!parseId(argument, id)) {
                std::cout << "Usage: " << command << " <id>\n";
                continue;
            }
            Student* student = gradeBook.find(id);
            if (!student) {
                std::cout << "Student not found.\n";
                continue;
            }

            if (command == "view") {
                printStudent(*student);
            } else if (command == "grade") {
                recordGradeInteractive(gradeBook, id);
                gradeBook.save();
            } else if (command == "ungrade") {
                removeGradeInteractive(gradeBook, id);
                gradeBook.save();
            } else {
                const auto confirm = lowercase(prompt("Remove '" + student->name + "'? [y/N]: "));
                if (confirm == "y" || confirm == "yes") {
                    gradeBook.remove(id);
                    gradeBook.save();
                    std::cout << "Student removed.\n";
                } else {
                    std::cout << "Removal cancelled.\n";
                }
            }
        } else if (command == "search") {
            if (argument.empty()) {
                std::cout << "Usage: search <text>\n";
            } else {
                listStudents(gradeBook.search(argument));
            }
        } else if (command == "stats") {
            printStats(gradeBook.computeStats());
        } else if (command == "quit" || command == "exit") {
            if (gradeBook.save()) {
                std::cout << "Grades saved. Goodbye.\n";
                return 0;
            }
            std::cerr << "Could not save " << gradeBook.dbFile() << ".\n";
            return 1;
        } else {
            std::cout << "Unknown command. Type 'help' for commands.\n";
        }
    }

    if (!gradeBook.save()) {
        std::cerr << "Could not save " << gradeBook.dbFile() << ".\n";
        return 1;
    }
    std::cout << "\nGrades saved. Goodbye.\n";
    return 0;
}

