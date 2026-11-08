#include "ui/ConsoleUI.hpp"

#include <iomanip>
#include <iostream>
#include <unistd.h>

namespace advshell {

void printHelp() {
    std::cout << "\nAdvanced Mini Shell Built-in Commands & Features:\n"
              << "  cd [dir]              Change current working directory\n"
              << "  pwd                   Print current working directory\n"
              << "  export KEY=VALUE      Set an environment variable\n"
              << "  unset KEY             Remove an environment variable\n"
              << "  alias [name=cmd]      List or define command aliases\n"
              << "  unalias <name>        Remove a command alias\n"
              << "  history               Display command history (use !! or !N to repeat)\n"
              << "  jobs                  List background jobs started with '&'\n"
              << "  clear / reset         Clear the terminal screen\n"
              << "  help                  Show this help\n"
              << "  exit / quit           Save history and exit shell\n\n"
              << "Shell Operators:\n"
              << "  cmd1 | cmd2           Pipe stdout of cmd1 into stdin of cmd2\n"
              << "  cmd > file            Redirect stdout to file (overwrite)\n"
              << "  cmd >> file           Redirect stdout to file (append)\n"
              << "  cmd < file            Redirect stdin from file\n"
              << "  cmd &                 Run command in the background\n"
              << "  $VAR / $?             Expand environment variable or last exit code\n\n";
}

std::string buildPrompt(int lastExitCode) {
    char cwd[1024];
    std::string dirName = "~";
    if (::getcwd(cwd, sizeof(cwd))) {
        std::string full(cwd);
        const auto slash = full.find_last_of('/');
        dirName = (slash == std::string::npos || slash + 1 >= full.size())
                      ? full
                      : full.substr(slash + 1);
    }
    const std::string statusMarker = (lastExitCode == 0) ? "" : (" [" + std::to_string(lastExitCode) + "]");
    return "mini-sh:" + dirName + statusMarker + "$ ";
}

void printHistory(const std::vector<std::string>& history) {
    if (history.empty()) {
        std::cout << "History is empty.\n";
        return;
    }
    for (std::size_t i = 0; i < history.size(); ++i) {
        std::cout << std::right << std::setw(4) << (i + 1) << "  " << history[i] << '\n';
    }
}

void printAliases(const std::map<std::string, std::string>& aliases) {
    if (aliases.empty()) {
        std::cout << "No aliases defined.\n";
        return;
    }
    for (const auto& [name, target] : aliases) {
        std::cout << "alias " << name << "='" << target << "'\n";
    }
}

void printJobs(const std::vector<BackgroundJob>& jobs) {
    if (jobs.empty()) {
        std::cout << "No background jobs.\n";
        return;
    }
    for (const auto& job : jobs) {
        std::cout << "[" << job.jobId << "] " << std::setw(8) << job.pid << "  "
                  << std::setw(10) << (job.running ? "Running" : "Done") << "  "
                  << job.command << '\n';
    }
}

} // namespace advshell

