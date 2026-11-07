#include "inventory/Inventory.hpp"

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <fstream>
#include <iomanip>

namespace invmgr {

namespace {

std::string escapeField(std::string_view value) {
    std::string result;
    for (const char c : value) {
        if (c == '\\') result += "\\\\";
        else if (c == '\t') result += "\\t";
        else if (c == '\n') result += "\\n";
        else result += c;
    }
    return result;
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
    for (const char c : line) {
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

bool parseIntField(std::string_view text, int& out) {
    auto res = std::from_chars(text.data(), text.data() + text.size(), out);
    return res.ec == std::errc() && res.ptr == text.data() + text.size();
}

} // namespace

Inventory::Inventory(std::string dbFile)
    : dbFile_(std::move(dbFile)) {}

bool Inventory::load() {
    std::ifstream input(dbFile_);
    if (!input) return false;

    std::string line;
    while (std::getline(input, line)) {
        const auto fields = splitFields(line);
        if (fields.size() != 7) continue;

        int id = 0;
        int quantity = 0;
        int reorderLevel = 0;
        if (!parseIntField(fields[0], id) || id < 1) continue;
        if (!parseIntField(fields[4], quantity) || quantity < 0) continue;
        if (!parseIntField(fields[5], reorderLevel) || reorderLevel < 0) continue;

        char* endPtr = nullptr;
        const double unitPrice = std::strtod(fields[6].c_str(), &endPtr);
        if (endPtr == fields[6].c_str() || *endPtr != '\0' || unitPrice < 0.0) {
            continue;
        }

        if (fields[2].empty()) continue;

        Product product{
            id,
            fields[1],
            fields[2],
            fields[3],
            quantity,
            reorderLevel,
            unitPrice
        };

        products_.push_back(std::move(product));
        nextId_ = std::max(nextId_, id + 1);
    }
    return true;
}

bool Inventory::save() const {
    std::ofstream output(dbFile_, std::ios::trunc);
    if (!output) return false;

    output << std::fixed << std::setprecision(2);
    for (const auto& p : products_) {
        output << p.id << '\t'
               << escapeField(p.sku) << '\t'
               << escapeField(p.name) << '\t'
               << escapeField(p.category) << '\t'
               << p.quantity << '\t'
               << p.reorderLevel << '\t'
               << p.unitPrice << '\n';
    }
    return static_cast<bool>(output);
}

Product& Inventory::add(Product product) {
    product.id = nextId_++;
    if (product.sku.empty()) {
        product.sku = "SKU-" + std::to_string(product.id);
    }
    products_.push_back(std::move(product));
    return products_.back();
}

Product* Inventory::find(int id) {
    for (auto& p : products_) {
        if (p.id == id) return &p;
    }
    return nullptr;
}

const Product* Inventory::find(int id) const {
    for (const auto& p : products_) {
        if (p.id == id) return &p;
    }
    return nullptr;
}

const Product* Inventory::findBySku(std::string_view sku) const {
    const auto target = lowercase(trim(sku));
    if (target.empty()) return nullptr;
    for (const auto& p : products_) {
        if (lowercase(p.sku) == target) return &p;
    }
    return nullptr;
}

bool Inventory::remove(int id) {
    const auto oldSize = products_.size();
    products_.erase(std::remove_if(products_.begin(), products_.end(),
                                   [id](const Product& p) { return p.id == id; }),
                    products_.end());
    return products_.size() != oldSize;
}

bool Inventory::restock(int id, int amount) {
    if (amount <= 0) return false;
    Product* p = find(id);
    if (!p) return false;
    p->quantity += amount;
    return true;
}

bool Inventory::sell(int id, int amount) {
    if (amount <= 0) return false;
    Product* p = find(id);
    if (!p || p->quantity < amount) return false;
    p->quantity -= amount;
    return true;
}

std::vector<const Product*> Inventory::lowStockProducts() const {
    std::vector<const Product*> result;
    for (const auto& p : products_) {
        if (p.isLowStock()) {
            result.push_back(&p);
        }
    }
    return result;
}

std::vector<const Product*> Inventory::filterByCategory(std::string_view category) const {
    const auto target = lowercase(trim(category));
    std::vector<const Product*> result;
    for (const auto& p : products_) {
        if (lowercase(p.category) == target) {
            result.push_back(&p);
        }
    }
    return result;
}

std::vector<const Product*> Inventory::search(std::string_view query) const {
    const auto queryLower = lowercase(trim(query));
    std::vector<const Product*> matches;
    if (queryLower.empty()) return matches;

    for (const auto& p : products_) {
        if (containsIgnoreCase(p.name, queryLower) ||
            containsIgnoreCase(p.sku, queryLower) ||
            containsIgnoreCase(p.category, queryLower)) {
            matches.push_back(&p);
        }
    }
    return matches;
}

InventorySummary Inventory::computeSummary() const {
    InventorySummary summary;
    summary.totalProducts = products_.size();
    for (const auto& p : products_) {
        summary.totalUnits += p.quantity;
        summary.totalValue += p.totalValue();
        if (p.quantity == 0) {
            ++summary.outOfStockCount;
        }
        if (p.isLowStock()) {
            ++summary.lowStockCount;
        }
    }
    return summary;
}

} // namespace invmgr

