#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace minidb {

struct Row {
    int id{};
    std::vector<std::string> values;
};

struct Table {
    std::string name;
    std::vector<std::string> columns;
    std::vector<Row> rows;
    int nextId{1};

    std::size_t columnIndex(std::string_view colName) const;
};

struct QueryResult {
    bool ok{true};
    std::string message;
    std::vector<std::string> columns;
    std::vector<Row> rows;
};

std::string_view trim(std::string_view text);
std::string lowercase(std::string_view text);
bool containsIgnoreCase(std::string_view value, std::string_view queryLower);

} // namespace minidb

