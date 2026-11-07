#pragma once

#include "Task.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace taskmgr {

class TaskManager {
public:
    explicit TaskManager(std::string dbFile = "tasks.db");

    bool load();
    bool save() const;

    Task& add(Task task);
    Task* find(int id);
    const Task* find(int id) const;
    bool remove(int id);

    const std::vector<Task>& tasks() const { return tasks_; }
    const std::string& dbFile() const { return dbFile_; }

private:
    std::string dbFile_;
    std::vector<Task> tasks_;
    int nextId_{1};
};

} // namespace taskmgr
