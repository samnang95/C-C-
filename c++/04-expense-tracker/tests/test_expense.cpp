#include "expense/Expense.hpp"
#include "expense/ExpenseTracker.hpp"
#include "ui/ConsoleUI.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>

int main() {
    using namespace exptrack;

    // 1. Date & string helpers
    assert(trim("  \t Coffee \n") == "Coffee");
    assert(lowercase("Food & Dining") == "food & dining");
    assert(isValidDate("2026-02-28"));
    assert(isValidDate("2024-02-29"));
    assert(!isValidDate("2026-02-29"));
    assert(!isValidDate("2026-13-01"));
    assert(isValidMonth("2026-11"));
    assert(!isValidMonth("2026-00"));
    assert(isValidDate(todayDate()));

    // 2. Parsing helpers
    int id = 0;
    assert(parseId("5", id) && id == 5);
    assert(!parseId("0", id));
    assert(!parseId("-3", id));

    double amount = 0.0;
    assert(parseAmount("19.99", amount) && std::abs(amount - 19.99) < 1e-6);
    assert(parseAmount("$42.50", amount) && std::abs(amount - 42.50) < 1e-6);
    assert(!parseAmount("0", amount));
    assert(!parseAmount("-5", amount));

    // 3. ExpenseTracker CRUD, filters, summary, and persistence
    const std::string testDb = "/tmp/test_expense_tracker.db";
    std::remove(testDb.c_str());

    {
        ExpenseTracker tracker(testDb);

        Expense e1{0, "2026-11-01", "Food", "Groceries\tStore", 85.50};
        Expense e2{0, "2026-11-03", "Transport", "Metro Pass", 25.00};
        Expense e3{0, "2026-10-28", "food", "Coffee & Pastry", 14.50};

        assert(tracker.add(e1).id == 1);
        assert(tracker.add(e2).id == 2);
        assert(tracker.add(e3).id == 3);

        const auto foodItems = tracker.filterByCategory("FOOD");
        assert(foodItems.size() == 2);

        const auto novItems = tracker.filterByMonth("2026-11");
        assert(novItems.size() == 2);

        const auto searchHits = tracker.search("metro");
        assert(searchHits.size() == 1 && searchHits[0]->id == 2);

        const auto allSummary = tracker.computeSummary();
        assert(allSummary.totalCount == 3);
        assert(std::abs(allSummary.totalAmount - 125.00) < 1e-6);
        assert(allSummary.byCategory.size() == 2);
        assert(std::abs(allSummary.byCategory[0].total - 100.00) < 1e-6);

        const auto novSummary = tracker.computeSummary("2026-11");
        assert(novSummary.totalCount == 2);
        assert(std::abs(novSummary.totalAmount - 110.50) < 1e-6);

        assert(tracker.save());
    }

    {
        ExpenseTracker loaded(testDb);
        assert(loaded.load());
        assert(loaded.expenses().size() == 3);

        const Expense* p1 = loaded.find(1);
        assert(p1 != nullptr);
        assert(p1->description == "Groceries\tStore");
        assert(std::abs(p1->amount - 85.50) < 1e-6);

        assert(loaded.remove(2));
        assert(!loaded.remove(2));
        assert(loaded.expenses().size() == 2);
    }

    std::remove(testDb.c_str());
    std::cout << "All expense tracker unit tests passed!\n";
    return 0;
}

