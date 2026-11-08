#include "shell/ShellEngine.hpp"
#include "ui/ConsoleUI.hpp"

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

namespace advshell {

ShellEngine::ShellEngine(std::string historyFile)
    : historyFile_(std::move(historyFile)) {
    aliases_["ll"] = "ls -la";
}

bool ShellEngine::loadHistory() {
    std::ifstream input(historyFile_);
    if (!input) return false;

    history_.clear();
    std::string line;
    while (std::getline(input, line)) {
        auto trimmed = trim(line);
        if (!trimmed.empty()) {
            history_.emplace_back(trimmed);
        }
    }
    return true;
}

bool ShellEngine::saveHistory() const {
    std::ofstream output(historyFile_, std::ios::trunc);
    if (!output) return false;

    const std::size_t start = history_.size() > 500 ? history_.size() - 500 : 0;
    for (std::size_t i = start; i < history_.size(); ++i) {
        output << history_[i] << '\n';
    }
    return static_cast<bool>(output);
}

void ShellEngine::reapBackgroundJobs() {
    for (auto& job : jobs_) {
        if (!job.running) continue;
        int status = 0;
        const pid_t result = ::waitpid(job.pid, &status, WNOHANG);
        if (result == job.pid || result < 0) {
            job.running = false;
        }
    }
}

bool ShellEngine::executeLine(std::string_view rawLine) {
    reapBackgroundJobs();

    auto trimmed = trim(rawLine);
    if (trimmed.empty()) return true;

    std::string effectiveLine(trimmed);

    // Handle !n or !! history expansion
    if (effectiveLine == "!!") {
        if (history_.empty()) {
            std::cout << "No commands in history.\n";
            lastExitCode_ = 1;
            return true;
        }
        effectiveLine = history_.back();
        std::cout << effectiveLine << '\n';
    } else if (effectiveLine.size() > 1 && effectiveLine.front() == '!') {
        std::size_t idx = 0;
        auto sub = std::string_view(effectiveLine).substr(1);
        auto res = std::from_chars(sub.data(), sub.data() + sub.size(), idx);
        if (res.ec == std::errc() && idx >= 1 && idx <= history_.size()) {
            effectiveLine = history_[idx - 1];
            std::cout << effectiveLine << '\n';
        }
    }

    if (history_.empty() || history_.back() != effectiveLine) {
        history_.push_back(effectiveLine);
    }

    const ParsedLine parsed = parseCommandLine(effectiveLine, aliases_, lastExitCode_);
    if (!parsed.errorMessage.empty()) {
        std::cerr << parsed.errorMessage << '\n';
        lastExitCode_ = 2;
        return true;
    }
    if (parsed.empty()) return true;

    // Single-stage built-in check (when no pipeline is present)
    if (parsed.pipeline.size() == 1 && !parsed.background &&
        parsed.pipeline[0].inputFile.empty() && parsed.pipeline[0].outputFile.empty()) {
        bool shouldExit = false;
        if (handleBuiltin(parsed.pipeline[0], shouldExit)) {
            return !shouldExit;
        }
    }

    lastExitCode_ = executeExternalPipeline(parsed);
    return true;
}

bool ShellEngine::handleBuiltin(const CommandStage& stage, bool& shouldExit) {
    const auto& args = stage.args;
    if (args.empty()) return false;
    const std::string& cmd = args[0];

    if (cmd == "exit" || cmd == "quit") {
        shouldExit = true;
        lastExitCode_ = 0;
        return true;
    }
    if (cmd == "help") {
        printHelp();
        lastExitCode_ = 0;
        return true;
    }
    if (cmd == "clear" || cmd == "reset" || cmd == "cls") {
        std::cout << "\033[2J\033[H" << std::flush;
        lastExitCode_ = 0;
        return true;
    }
    if (cmd == "pwd") {
        char cwd[4096];
        if (::getcwd(cwd, sizeof(cwd))) {
            std::cout << cwd << '\n';
            lastExitCode_ = 0;
        } else {
            lastExitCode_ = 1;
        }
        return true;
    }
    if (cmd == "cd") {
        const char* target = (args.size() > 1) ? args[1].c_str() : std::getenv("HOME");
        if (!target || ::chdir(target) != 0) {
            std::cerr << "cd: could not change directory\n";
            lastExitCode_ = 1;
        } else {
            lastExitCode_ = 0;
        }
        return true;
    }
    if (cmd == "history") {
        printHistory(history_);
        lastExitCode_ = 0;
        return true;
    }
    if (cmd == "jobs") {
        reapBackgroundJobs();
        printJobs(jobs_);
        lastExitCode_ = 0;
        return true;
    }
    if (cmd == "alias") {
        if (args.size() == 1) {
            printAliases(aliases_);
            lastExitCode_ = 0;
            return true;
        }
        for (std::size_t i = 1; i < args.size(); ++i) {
            const auto eq = args[i].find('=');
            if (eq != std::string::npos && eq > 0) {
                aliases_[args[i].substr(0, eq)] = args[i].substr(eq + 1);
            }
        }
        lastExitCode_ = 0;
        return true;
    }
    if (cmd == "unalias") {
        for (std::size_t i = 1; i < args.size(); ++i) {
            aliases_.erase(args[i]);
        }
        lastExitCode_ = 0;
        return true;
    }
    if (cmd == "export") {
        for (std::size_t i = 1; i < args.size(); ++i) {
            const auto eq = args[i].find('=');
            if (eq != std::string::npos && eq > 0) {
                const std::string key = args[i].substr(0, eq);
                const std::string val = args[i].substr(eq + 1);
                ::setenv(key.c_str(), val.c_str(), 1);
            }
        }
        lastExitCode_ = 0;
        return true;
    }
    if (cmd == "unset") {
        for (std::size_t i = 1; i < args.size(); ++i) {
            ::unsetenv(args[i].c_str());
        }
        lastExitCode_ = 0;
        return true;
    }

    return false;
}

int ShellEngine::executeExternalPipeline(const ParsedLine& parsed) {
    const std::size_t numStages = parsed.pipeline.size();
    std::vector<pid_t> pids;
    pids.reserve(numStages);

    int prevReadFd = -1;

    for (std::size_t i = 0; i < numStages; ++i) {
        int pipeFds[2] = {-1, -1};
        if (i + 1 < numStages) {
            if (::pipe(pipeFds) < 0) {
                std::cerr << "pipe failed\n";
                if (prevReadFd >= 0) ::close(prevReadFd);
                return 1;
            }
        }

        const pid_t pid = ::fork();
        if (pid < 0) {
            std::cerr << "fork failed\n";
            if (prevReadFd >= 0) ::close(prevReadFd);
            if (pipeFds[0] >= 0) ::close(pipeFds[0]);
            if (pipeFds[1] >= 0) ::close(pipeFds[1]);
            return 1;
        }

        if (pid == 0) {
            // Child process
            if (prevReadFd >= 0) {
                ::dup2(prevReadFd, STDIN_FILENO);
                ::close(prevReadFd);
            }
            if (pipeFds[1] >= 0) {
                ::dup2(pipeFds[1], STDOUT_FILENO);
                ::close(pipeFds[1]);
            }
            if (pipeFds[0] >= 0) {
                ::close(pipeFds[0]);
            }

            const CommandStage& stage = parsed.pipeline[i];
            if (!stage.inputFile.empty()) {
                const int inFd = ::open(stage.inputFile.c_str(), O_RDONLY);
                if (inFd < 0) {
                    std::cerr << "Cannot open input file: " << stage.inputFile << '\n';
                    std::_Exit(1);
                }
                ::dup2(inFd, STDIN_FILENO);
                ::close(inFd);
            }
            if (!stage.outputFile.empty()) {
                const int flags = O_WRONLY | O_CREAT | (stage.appendOutput ? O_APPEND : O_TRUNC);
                const int outFd = ::open(stage.outputFile.c_str(), flags, 0644);
                if (outFd < 0) {
                    std::cerr << "Cannot open output file: " << stage.outputFile << '\n';
                    std::_Exit(1);
                }
                ::dup2(outFd, STDOUT_FILENO);
                ::close(outFd);
            }

            std::vector<char*> argv;
            argv.reserve(stage.args.size() + 1);
            for (const auto& arg : stage.args) {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }
            argv.push_back(nullptr);

            ::execvp(argv[0], argv.data());
            std::cerr << stage.args[0] << ": command not found\n";
            std::_Exit(127);
        }

        // Parent process
        pids.push_back(pid);
        if (prevReadFd >= 0) {
            ::close(prevReadFd);
        }
        if (pipeFds[1] >= 0) {
            ::close(pipeFds[1]);
        }
        prevReadFd = pipeFds[0];
    }

    if (parsed.background && !pids.empty()) {
        std::string cmdSummary = parsed.pipeline.front().args.front();
        const int jobId = nextJobId_++;
        jobs_.push_back(BackgroundJob{jobId, static_cast<int>(pids.back()), cmdSummary, true});
        std::cout << "[" << jobId << "] " << pids.back() << '\n';
        return 0;
    }

    int lastStatus = 0;
    for (const pid_t cpid : pids) {
        int status = 0;
        ::waitpid(cpid, &status, 0);
        if (WIFEXITED(status)) {
            lastStatus = WEXITSTATUS(status);
        } else {
            lastStatus = 1;
        }
    }
    return lastStatus;
}

} // namespace advshell

