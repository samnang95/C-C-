#pragma once

#include "inventory/Product.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace invmgr {

struct InventorySummary {
    std::size_t totalProducts{};
    long long totalUnits{};
    std::size_t lowStockCount{};
    std::size_t outOfStockCount{};
    double totalValue{};
};

class Inventory {
public:
    explicit Inventory(std::string dbFile = "data/inventory.db");

    bool load();
    bool save() const;

    Product& add(Product product);
    Product* find(int id);
    const Product* find(int id) const;
    const Product* findBySku(std::string_view sku) const;
    bool remove(int id);

    bool restock(int id, int amount);
    bool sell(int id, int amount);

    std::vector<const Product*> lowStockProducts() const;
    std::vector<const Product*> filterByCategory(std::string_view category) const;
    std::vector<const Product*> search(std::string_view query) const;
    InventorySummary computeSummary() const;

    const std::vector<Product>& products() const { return products_; }
    const std::string& dbFile() const { return dbFile_; }

private:
    std::string dbFile_;
    std::vector<Product> products_;
    int nextId_{1};
};

} // namespace invmgr

