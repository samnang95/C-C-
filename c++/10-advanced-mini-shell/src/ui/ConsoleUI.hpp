#pragma once

#include "shell/ShellEngine.hpp"

#include <map>
#include <string>
#include <vector>

namespace advshell {

void printHelp();
std::string buildPrompt(int lastExitCode);
void printHistory(const std::vector<std::string>& history);
void printAliases(const std::map<std::string, std::string>& aliases);
void printJobs(const std::vector<BackgroundJob>& jobs);

} // namespace advshell

