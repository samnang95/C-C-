#pragma once

#include "expense/Expense.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace exptrack {

struct CategoryTotal {
    std::string category;
    std::size_t count{};
    double total{};
};

struct ExpenseSummary {
    std::size_t totalCount{};
    double totalAmount{};
    double averageAmount{};
    double highestAmount{};
    std::string highestDescription;
    std::vector<CategoryTotal> byCategory;
};

class ExpenseTracker {
public:
    explicit ExpenseTracker(std::string dbFile = "data/expenses.db");

    bool load();
    bool save() const;

    Expense& add(Expense expense);
    Expense* find(int id);
    const Expense* find(int id) const;
    bool remove(int id);

    std::vector<const Expense*> filterByCategory(std::string_view category) const;
    std::vector<const Expense*> filterByMonth(std::string_view yearMonth) const;
    std::vector<const Expense*> search(std::string_view query) const;
    ExpenseSummary computeSummary(std::string_view yearMonth = "") const;

    const std::vector<Expense>& expenses() const { return expenses_; }
    const std::string& dbFile() const { return dbFile_; }

private:
    std::string dbFile_;
    std::vector<Expense> expenses_;
    int nextId_{1};
};

} // namespace exptrack

