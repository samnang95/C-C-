#pragma once

#include "shell/CommandParser.hpp"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace advshell {

struct BackgroundJob {
    int jobId{};
    int pid{};
    std::string command;
    bool running{true};
};

class ShellEngine {
public:
    explicit ShellEngine(std::string historyFile = "data/shell_history.db");

    bool loadHistory();
    bool saveHistory() const;

    // Executes a command line and returns true to continue REPL, false on exit
    bool executeLine(std::string_view line);

    int lastExitCode() const { return lastExitCode_; }
    const std::vector<std::string>& history() const { return history_; }
    const std::map<std::string, std::string>& aliases() const { return aliases_; }
    const std::vector<BackgroundJob>& jobs() { reapBackgroundJobs(); return jobs_; }

private:
    bool handleBuiltin(const CommandStage& stage, bool& shouldExit);
    int executeExternalPipeline(const ParsedLine& parsed);
    void reapBackgroundJobs();

    std::string historyFile_;
    std::vector<std::string> history_;
    std::map<std::string, std::string> aliases_;
    std::vector<BackgroundJob> jobs_;
    int nextJobId_{1};
    int lastExitCode_{0};
};

} // namespace advshell

