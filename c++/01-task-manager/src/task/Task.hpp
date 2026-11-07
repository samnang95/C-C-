#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace taskmgr {

enum class Priority { Low, Medium, High };

struct Task {
    int id{};
    std::string title;
    std::string description;
    Priority priority{Priority::Medium};
    std::string dueDate;
    std::vector<std::string> tags;
    bool completed{};
};

std::string_view trim(std::string_view text);
std::string lowercase(std::string_view text);
std::string priorityName(Priority priority);
Priority parsePriority(std::string_view text);
bool isValidDate(std::string_view date);
std::vector<std::string> splitTags(std::string_view line);
std::string joinTags(const std::vector<std::string>& tags, const char* separator = ", ");

} // namespace taskmgr
