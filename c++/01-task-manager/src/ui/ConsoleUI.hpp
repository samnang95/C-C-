#pragma once

#include "task/Task.hpp"
#include "task/TaskManager.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace taskmgr {

void printHelp();
void printTask(const Task& task);
void listTasks(const std::vector<const Task*>& tasks, std::string_view filter = "");
void listTasks(const std::vector<Task>& tasks, std::string_view filter = "");
bool parseId(std::string_view value, int& id);
std::string prompt(std::string_view label);
void addTask(TaskManager& manager);

} // namespace taskmgr
