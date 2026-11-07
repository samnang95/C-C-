#pragma once

#include <string>
#include <string_view>

namespace exptrack {

struct Expense {
    int id{};
    std::string date;        // YYYY-MM-DD
    std::string category;
    std::string description;
    double amount{};
};

std::string_view trim(std::string_view text);
std::string lowercase(std::string_view text);
bool containsIgnoreCase(std::string_view value, std::string_view queryLower);
bool isValidDate(std::string_view date);
bool isValidMonth(std::string_view month);
std::string todayDate();

} // namespace exptrack

