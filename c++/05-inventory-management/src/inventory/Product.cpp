#include "inventory/Product.hpp"

#include <algorithm>
#include <cctype>

namespace invmgr {

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

} // namespace invmgr

