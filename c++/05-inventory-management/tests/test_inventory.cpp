#include "inventory/Inventory.hpp"
#include "inventory/Product.hpp"
#include "ui/ConsoleUI.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>

int main() {
    using namespace invmgr;

    // 1. Helpers
    assert(trim("  Mechanical Keyboard \t\n") == "Mechanical Keyboard");
    assert(lowercase("KB-100") == "kb-100");

    int id = 0;
    assert(parseId("8", id) && id == 8);
    assert(!parseId("0", id));
    assert(!parseId("-1", id));

    int qty = 0;
    assert(parseNonNegativeInt("0", qty) && qty == 0);
    assert(parseNonNegativeInt("25", qty) && qty == 25);
    assert(!parseNonNegativeInt("-4", qty));

    double price = 0.0;
    assert(parsePrice("$129.99", price) && std::abs(price - 129.99) < 1e-6);
    assert(parsePrice("0", price) && std::abs(price) < 1e-6);
    assert(!parsePrice("-10", price));

    // 2. Inventory CRUD, stock operations, filters, summary, and persistence
    const std::string testDb = "/tmp/test_inventory_mgmt.db";
    std::remove(testDb.c_str());

    {
        Inventory inv(testDb);

        Product p1{0, "KB-01", "Mechanical\tKeyboard", "Peripherals", 10, 3, 120.00};
        Product p2{0, "", "USB-C Cable", "Accessories", 2, 5, 15.00};

        assert(inv.add(p1).id == 1);
        const auto& added2 = inv.add(p2);
        assert(added2.id == 2);
        assert(added2.sku == "SKU-2");

        assert(inv.findBySku("kb-01") != nullptr);

        assert(inv.restock(1, 5));
        assert(inv.find(1)->quantity == 15);

        assert(inv.sell(1, 4));
        assert(inv.find(1)->quantity == 11);
        assert(!inv.sell(1, 100)); // insufficient stock

        const auto lowStock = inv.lowStockProducts();
        assert(lowStock.size() == 1 && lowStock[0]->id == 2);

        const auto catMatches = inv.filterByCategory("peripherals");
        assert(catMatches.size() == 1 && catMatches[0]->id == 1);

        const auto searchHits = inv.search("usb-c");
        assert(searchHits.size() == 1 && searchHits[0]->id == 2);

        const auto summary = inv.computeSummary();
        assert(summary.totalProducts == 2);
        assert(summary.totalUnits == 13);
        assert(summary.lowStockCount == 1);
        assert(std::abs(summary.totalValue - (11 * 120.00 + 2 * 15.00)) < 1e-6);

        assert(inv.save());
    }

    {
        Inventory loaded(testDb);
        assert(loaded.load());
        assert(loaded.products().size() == 2);

        const Product* p1 = loaded.find(1);
        assert(p1 != nullptr);
        assert(p1->name == "Mechanical\tKeyboard");
        assert(p1->quantity == 11);

        assert(loaded.remove(2));
        assert(!loaded.remove(2));
        assert(loaded.products().size() == 1);
    }

    std::remove(testDb.c_str());
    std::cout << "All inventory management unit tests passed!\n";
    return 0;
}

