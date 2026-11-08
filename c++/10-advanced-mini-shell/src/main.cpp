#include "shell/CommandParser.hpp"
#include "shell/ShellEngine.hpp"
#include "ui/ConsoleUI.hpp"

#include <csignal>
#include <iostream>
#include <string>

int main() {
    using namespace advshell;

    std::signal(SIGINT, SIG_IGN);

    ShellEngine shell;
    shell.loadHistory();
    std::cout << "Advanced Mini Shell (C++17) — type 'help' for built-ins and operators.\n";

    std::string line;
    while (std::cout << buildPrompt(shell.lastExitCode()) && std::getline(std::cin, line)) {
        if (!shell.executeLine(line)) {
            break;
        }
    }

    shell.saveHistory();
    std::cout << "Goodbye.\n";
    return 0;
}

