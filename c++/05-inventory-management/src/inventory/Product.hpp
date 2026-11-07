#pragma once

#include <string>
#include <string_view>

namespace invmgr {

struct Product {
    int id{};
    std::string sku;
    std::string name;
    std::string category;
    int quantity{};
    int reorderLevel{5};
    double unitPrice{};

    double totalValue() const { return static_cast<double>(quantity) * unitPrice; }
    bool isLowStock() const { return quantity <= reorderLevel; }
};

std::string_view trim(std::string_view text);
std::string lowercase(std::string_view text);
bool containsIgnoreCase(std::string_view value, std::string_view queryLower);

} // namespace invmgr

