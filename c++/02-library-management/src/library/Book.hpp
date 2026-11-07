#pragma once

#include <string>
#include <string_view>

namespace libmgr {

struct Book {
    int id{};
    std::string title;
    std::string author;
    std::string isbn;
    std::string category;
    bool borrowed{};
    std::string borrower;
};

std::string_view trim(std::string_view text);
std::string lowercase(std::string_view text);
bool containsIgnoreCase(std::string_view value, std::string_view queryLower);

} // namespace libmgr

