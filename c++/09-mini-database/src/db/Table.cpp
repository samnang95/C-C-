#include "db/Table.hpp"

#include <algorithm>
#include <cctype>

namespace minidb {

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

std::size_t Table::columnIndex(std::string_view colName) const {
    const auto target = lowercase(trim(colName));
    for (std::size_t i = 0; i < columns.size(); ++i) {
        if (lowercase(columns[i]) == target) {
            return i;
        }
    }
    return columns.size();
}

} // namespace minidb

