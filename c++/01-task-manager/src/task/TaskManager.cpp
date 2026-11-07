#include "task/TaskManager.hpp"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <sstream>

namespace taskmgr {

namespace {

std::string escapeField(std::string_view value) {
    std::string escaped;
    for (char c : value) {
        if (c == '\\') escaped += "\\\\";
        else if (c == '\t') escaped += "\\t";
        else if (c == '\n') escaped += "\\n";
        else escaped += c;
    }
    return escaped;
}

std::string unescapeField(std::string_view value) {
    std::string result;
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '\\' && i + 1 < value.size()) {
            const char next = value[++i];
            result += (next == 't') ? '\t' : ((next == 'n') ? '\n' : next);
        } else {
            result += value[i];
        }
    }
    return result;
}

std::vector<std::string> splitFields(std::string_view line) {
    std::vector<std::string> fields;
    std::string field;
    bool escaped = false;
    for (char c : line) {
        if (escaped) {
            field += '\\';
            field += c;
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == '\t') {
            fields.push_back(unescapeField(field));
            field.clear();
        } else {
            field += c;
        }
    }
    if (escaped) field += '\\';
    fields.push_back(unescapeField(field));
    return fields;
}

} // namespace

TaskManager::TaskManager(std::string dbFile)
    : dbFile_(std::move(dbFile)) {}

bool TaskManager::load() {
    std::ifstream input(dbFile_);
    if (!input) return false;

    std::string line;
    while (std::getline(input, line)) {
        const auto fields = splitFields(line);
        if (fields.size() != 7) continue;

        int id = 0;
        auto res = std::from_chars(fields[0].data(), fields[0].data() + fields[0].size(), id);
        if (res.ec != std::errc()) continue;

        Task task;
        task.id = id;
        task.title = fields[1];
        task.description = fields[2];
        task.priority = parsePriority(fields[3]);
        task.dueDate = fields[4];
        task.tags = splitTags(fields[5]);
        task.completed = (fields[6] == "1");

        tasks_.push_back(std::move(task));
        nextId_ = std::max(nextId_, id + 1);
    }
    return true;
}

bool TaskManager::save() const {
    std::ofstream output(dbFile_, std::ios::trunc);
    if (!output) return false;

    for (const auto& task : tasks_) {
        output << task.id << '\t'
               << escapeField(task.title) << '\t'
               << escapeField(task.description) << '\t'
               << priorityName(task.priority) << '\t'
               << escapeField(task.dueDate) << '\t'
               << escapeField(joinTags(task.tags, ",")) << '\t'
               << (task.completed ? "1" : "0") << '\n';
    }
    return static_cast<bool>(output);
}

Task& TaskManager::add(Task task) {
    task.id = nextId_++;
    tasks_.push_back(std::move(task));
    return tasks_.back();
}

Task* TaskManager::find(int id) {
    for (auto& task : tasks_) {
        if (task.id == id) return &task;
    }
    return nullptr;
}

const Task* TaskManager::find(int id) const {
    for (const auto& task : tasks_) {
        if (task.id == id) return &task;
    }
    return nullptr;
}

bool TaskManager::remove(int id) {
    const auto oldSize = tasks_.size();
    tasks_.erase(std::remove_if(tasks_.begin(), tasks_.end(),
                                [id](const Task& t) { return t.id == id; }),
                tasks_.end());
    return tasks_.size() != oldSize;
}

} // namespace taskmgr
