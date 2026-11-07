#pragma once

#include "inventory/Inventory.hpp"
#include "inventory/Product.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace invmgr {

void printHelp();
void printProduct(const Product& product);
void listProducts(const std::vector<const Product*>& products);
void listProducts(const std::vector<Product>& products);
void printSummary(const InventorySummary& summary);

bool parseId(std::string_view text, int& id);
bool parseNonNegativeInt(std::string_view text, int& value);
bool parsePrice(std::string_view text, double& price);
std::string prompt(std::string_view label);

void addProductInteractive(Inventory& inventory);
void editProductInteractive(Inventory& inventory, int id);
void adjustStockInteractive(Inventory& inventory, int id, bool isRestock);

} // namespace invmgr

