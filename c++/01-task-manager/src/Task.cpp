#include "Task.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <sstream>

namespace taskmgr {

std::string_view trim(std::string_view text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) return {};
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

std::string lowercase(std::string_view text) {
    std::string result(text);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

std::string priorityName(Priority priority) {
    switch (priority) {
        case Priority::Low: return "low";
        case Priority::High: return "high";
        default: return "medium";
    }
}

Priority parsePriority(std::string_view text) {
    const auto value = lowercase(trim(text));
    if (value == "low") return Priority::Low;
    if (value == "high") return Priority::High;
    return Priority::Medium;
}

bool isValidDate(std::string_view date) {
    if (date.empty()) return true;
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
    for (std::size_t i = 0; i < date.size(); ++i) {
        if (i != 4 && i != 7 && !std::isdigit(static_cast<unsigned char>(date[i]))) return false;
    }

    int year = 0;
    int month = 0;
    int day = 0;

    auto r1 = std::from_chars(date.data(), date.data() + 4, year);
    auto r2 = std::from_chars(date.data() + 5, date.data() + 7, month);
    auto r3 = std::from_chars(date.data() + 8, date.data() + 10, day);

    if (r1.ec != std::errc() || r2.ec != std::errc() || r3.ec != std::errc()) {
        return false;
    }

    if (year < 1 || month < 1 || month > 12 || day < 1) return false;
    static const int daysPerMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    const bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    const int maxDay = (month == 2 && leap) ? 29 : daysPerMonth[month - 1];
    return day <= maxDay;
}

std::vector<std::string> splitTags(std::string_view line) {
    std::vector<std::string> tags;
    std::string lineStr(line);
    std::stringstream stream(lineStr);
    std::string item;
    while (std::getline(stream, item, ',')) {
        auto trimmed = trim(item);
        if (!trimmed.empty()) tags.emplace_back(trimmed);
    }
    return tags;
}

std::string joinTags(const std::vector<std::string>& tags, const char* separator) {
    std::ostringstream output;
    for (std::size_t i = 0; i < tags.size(); ++i) {
        if (i) output << separator;
        output << tags[i];
    }
    return output.str();
}

} // namespace taskmgr
