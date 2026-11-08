#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace advshell {

struct CommandStage {
    std::vector<std::string> args;
    std::string inputFile;
    std::string outputFile;
    bool appendOutput{false};
};

struct ParsedLine {
    std::vector<CommandStage> pipeline;
    bool background{false};
    std::string errorMessage;

    bool empty() const { return pipeline.empty() || pipeline.front().args.empty(); }
};

std::string_view trim(std::string_view text);
std::string lowercase(std::string_view text);
std::string expandVariables(std::string_view input, int lastExitCode = 0);
std::vector<std::string> tokenize(std::string_view input);
ParsedLine parseCommandLine(std::string_view input,
                            const std::map<std::string, std::string>& aliases = {},
                            int lastExitCode = 0);

} // namespace advshell

