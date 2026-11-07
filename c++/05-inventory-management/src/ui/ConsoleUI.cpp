#include "ui/ConsoleUI.hpp"

#include <charconv>
#include <cstdlib>
#include <iomanip>
#include <iostream>

namespace invmgr {

void printHelp() {
    std::cout << "\nCommands:\n"
              << "  add                   Add a new product\n"
              << "  list [low|<category>] List all products (or low-stock / category)\n"
              << "  view [id]             Show full product details\n"
              << "  edit [id]             Edit product details\n"
              << "  restock [id]          Add stock quantity to a product\n"
              << "  sell [id]             Deduct sold quantity from a product\n"
              << "  remove [id]           Delete a product\n"
              << "  search <text>         Search by name, SKU, or category\n"
              << "  summary               Show total inventory valuation & alerts\n"
              << "  clear / reset         Clear the terminal screen\n"
              << "  help                  Show this help\n"
              << "  quit                  Save and exit\n\n";
}

void printProduct(const Product& product) {
    std::cout << std::fixed << std::setprecision(2)
              << "\n" << product.name << " (ID " << product.id << ")\n"
              << "  SKU:           " << product.sku << '\n'
              << "  Category:      " << (product.category.empty() ? "uncategorized" : product.category) << '\n'
              << "  Quantity:      " << product.quantity
              << (product.isLowStock() ? " [LOW STOCK]" : "") << '\n'
              << "  Reorder level: " << product.reorderLevel << '\n'
              << "  Unit price:    $" << product.unitPrice << '\n'
              << "  Total value:   $" << product.totalValue() << "\n\n";
}

void listProducts(const std::vector<const Product*>& products) {
    if (products.empty()) {
        std::cout << "\nNo matching products.\n\n";
        return;
    }

    std::cout << "\n" << std::left << std::setw(5) << "ID"
              << std::setw(12) << "SKU"
              << std::setw(24) << "Name"
              << std::setw(15) << "Category"
              << std::right << std::setw(7) << "Qty"
              << std::setw(11) << "Price"
              << std::setw(12) << "Value"
              << "  Status\n"
              << std::string(96, '-') << '\n';

    std::cout << std::fixed << std::setprecision(2);
    for (const auto* p : products) {
        if (!p) continue;
        const char* status = (p->quantity == 0) ? "OUT" : (p->isLowStock() ? "LOW" : "OK");
        std::cout << std::left << std::setw(5) << p->id
                  << std::setw(12) << p->sku.substr(0, 11)
                  << std::setw(24) << p->name.substr(0, 23)
                  << std::setw(15) << (p->category.empty() ? "-" : p->category.substr(0, 14))
                  << std::right << std::setw(7) << p->quantity
                  << std::setw(11) << p->unitPrice
                  << std::setw(12) << p->totalValue()
                  << "  " << status << '\n';
    }
    std::cout << '\n';
}

void listProducts(const std::vector<Product>& products) {
    std::vector<const Product*> ptrs;
    ptrs.reserve(products.size());
    for (const auto& p : products) {
        ptrs.push_back(&p);
    }
    listProducts(ptrs);
}

void printSummary(const InventorySummary& summary) {
    std::cout << std::fixed << std::setprecision(2)
              << "\nInventory Summary:\n"
              << "  Total products: " << summary.totalProducts << '\n'
              << "  Total units:    " << summary.totalUnits << '\n'
              << "  Low stock:      " << summary.lowStockCount << '\n'
              << "  Out of stock:   " << summary.outOfStockCount << '\n'
              << "  Total value:    $" << summary.totalValue << "\n\n";
}

bool parseId(std::string_view text, int& id) {
    auto trimmed = trim(text);
    if (trimmed.empty()) return false;

    int parsed = 0;
    auto res = std::from_chars(trimmed.data(), trimmed.data() + trimmed.size(), parsed);
    if (res.ec != std::errc() || res.ptr != trimmed.data() + trimmed.size() || parsed <= 0) {
        return false;
    }
    id = parsed;
    return true;
}

bool parseNonNegativeInt(std::string_view text, int& value) {
    auto trimmed = trim(text);
    if (trimmed.empty()) return false;

    int parsed = 0;
    auto res = std::from_chars(trimmed.data(), trimmed.data() + trimmed.size(), parsed);
    if (res.ec != std::errc() || res.ptr != trimmed.data() + trimmed.size() || parsed < 0) {
        return false;
    }
    value = parsed;
    return true;
}

bool parsePrice(std::string_view text, double& price) {
    std::string trimmed(trim(text));
    if (!trimmed.empty() && trimmed.front() == '$') {
        trimmed = std::string(trim(std::string_view(trimmed).substr(1)));
    }
    if (trimmed.empty()) return false;

    char* endPtr = nullptr;
    const double parsed = std::strtod(trimmed.c_str(), &endPtr);
    if (endPtr == trimmed.c_str() || *endPtr != '\0' || parsed < 0.0) {
        return false;
    }
    price = parsed;
    return true;
}

std::string prompt(std::string_view label) {
    std::cout << label;
    std::string value;
    if (!std::getline(std::cin, value)) {
        return "";
    }
    return std::string(trim(value));
}

void addProductInteractive(Inventory& inventory) {
    Product p;

    while (std::cin) {
        p.name = prompt("Product name: ");
        if (!std::cin) return;
        if (p.name.empty()) {
            std::cout << "A product name is required.\n";
            continue;
        }
        break;
    }

    p.sku = prompt("SKU (optional, auto-generated if blank): ");
    if (!std::cin) return;

    p.category = prompt("Category (optional): ");
    if (!std::cin) return;

    while (std::cin) {
        const auto raw = prompt("Initial quantity (0): ");
        if (!std::cin) return;
        if (raw.empty()) {
            p.quantity = 0;
            break;
        }
        if (parseNonNegativeInt(raw, p.quantity)) break;
        std::cout << "Please enter a non-negative integer.\n";
    }

    while (std::cin) {
        const auto raw = prompt("Reorder alert level (5): ");
        if (!std::cin) return;
        if (raw.empty()) {
            p.reorderLevel = 5;
            break;
        }
        if (parseNonNegativeInt(raw, p.reorderLevel)) break;
        std::cout << "Please enter a non-negative integer.\n";
    }

    while (std::cin) {
        const auto raw = prompt("Unit price ($): ");
        if (!std::cin) return;
        if (parsePrice(raw, p.unitPrice)) break;
        std::cout << "Please enter a valid price.\n";
    }

    const auto& added = inventory.add(std::move(p));
    std::cout << "Added product " << added.id << " (" << added.sku << ").\n";
}

void editProductInteractive(Inventory& inventory, int id) {
    Product* p = inventory.find(id);
    if (!p) {
        std::cout << "Product not found.\n";
        return;
    }

    const auto name = prompt("Name [" + p->name + "]: ");
    if (!std::cin) return;
    if (!name.empty()) p->name = name;

    const auto sku = prompt("SKU [" + p->sku + "]: ");
    if (!std::cin) return;
    if (!sku.empty()) p->sku = sku;

    const auto cat = prompt("Category [" + p->category + "]: ");
    if (!std::cin) return;
    if (!cat.empty()) p->category = cat;

    while (std::cin) {
        const auto rawPrice = prompt("Unit price [" + std::to_string(p->unitPrice) + "]: ");
        if (!std::cin) return;
        if (rawPrice.empty()) break;
        double parsed = 0.0;
        if (parsePrice(rawPrice, parsed)) {
            p->unitPrice = parsed;
            break;
        }
        std::cout << "Please enter a valid price.\n";
    }

    while (std::cin) {
        const auto rawReorder = prompt("Reorder level [" + std::to_string(p->reorderLevel) + "]: ");
        if (!std::cin) return;
        if (rawReorder.empty()) break;
        int parsed = 0;
        if (parseNonNegativeInt(rawReorder, parsed)) {
            p->reorderLevel = parsed;
            break;
        }
        std::cout << "Please enter a non-negative integer.\n";
    }

    std::cout << "Product " << p->id << " updated.\n";
}

void adjustStockInteractive(Inventory& inventory, int id, bool isRestock) {
    Product* p = inventory.find(id);
    if (!p) {
        std::cout << "Product not found.\n";
        return;
    }

    int amount = 0;
    const auto raw = prompt(std::string(isRestock ? "Quantity to add: " : "Quantity to sell: "));
    if (!std::cin) return;
    if (!parseId(raw, amount)) {
        std::cout << "Please enter a positive integer.\n";
        return;
    }

    if (isRestock) {
        inventory.restock(id, amount);
        std::cout << "Restocked " << p->name << ". New quantity: " << p->quantity << ".\n";
    } else {
        if (!inventory.sell(id, amount)) {
            std::cout << "Insufficient stock (available: " << p->quantity << ").\n";
            return;
        }
        std::cout << "Sold " << amount << " of " << p->name << ". Remaining: " << p->quantity << ".\n";
        if (p->isLowStock()) {
            std::cout << "Warning: " << p->name << " is at or below reorder level (" << p->reorderLevel << ")!\n";
        }
    }
}

} // namespace invmgr

