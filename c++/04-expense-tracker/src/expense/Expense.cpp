#include "expense/Expense.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace exptrack {

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

bool containsIgnoreCase(std::string_view value, std::string_view queryLower) {
    return lowercase(value).find(queryLower) != std::string::npos;
}

bool isValidDate(std::string_view date) {
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

    if (r1.ec != std::errc() || r2.ec != std::errc() || r3.ec != std::errc()) return false;
    if (year < 1900 || year > 2100 || month < 1 || month > 12 || day < 1) return false;

    static const int daysPerMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    const bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    const int maxDay = (month == 2 && leap) ? 29 : daysPerMonth[month - 1];
    return day <= maxDay;
}

bool isValidMonth(std::string_view month) {
    if (month.size() != 7 || month[4] != '-') return false;
    for (std::size_t i = 0; i < month.size(); ++i) {
        if (i != 4 && !std::isdigit(static_cast<unsigned char>(month[i]))) return false;
    }
    int y = 0;
    int m = 0;
    auto r1 = std::from_chars(month.data(), month.data() + 4, y);
    auto r2 = std::from_chars(month.data() + 5, month.data() + 7, m);
    return r1.ec == std::errc() && r2.ec == std::errc() && y >= 1900 && y <= 2100 && m >= 1 && m <= 12;
}

std::string todayDate() {
    std::time_t now = std::time(nullptr);
    std::tm tmValue{};
#if defined(_WIN32)
    localtime_s(&tmValue, &now);
#else
    localtime_r(&now, &tmValue);
#endif
    std::ostringstream out;
    out << std::put_time(&tmValue, "%Y-%m-%d");
    return out.str();
}

} // namespace exptrack

