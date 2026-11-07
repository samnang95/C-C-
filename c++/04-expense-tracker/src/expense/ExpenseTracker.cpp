#include "expense/ExpenseTracker.hpp"

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <map>

namespace exptrack {

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

} // namespace

ExpenseTracker::ExpenseTracker(std::string dbFile)
    : dbFile_(std::move(dbFile)) {}

bool ExpenseTracker::load() {
    std::ifstream input(dbFile_);
    if (!input) return false;

    std::string line;
    while (std::getline(input, line)) {
        const auto fields = splitFields(line);
        if (fields.size() != 5) continue;

        int id = 0;
        auto res = std::from_chars(fields[0].data(), fields[0].data() + fields[0].size(), id);
        if (res.ec != std::errc() || res.ptr != fields[0].data() + fields[0].size() || id < 1) {
            continue;
        }

        char* endPtr = nullptr;
        const double amount = std::strtod(fields[4].c_str(), &endPtr);
        if (endPtr == fields[4].c_str() || *endPtr != '\0' || amount <= 0.0) {
            continue;
        }

        if (!isValidDate(fields[1]) || fields[2].empty() || fields[3].empty()) {
            continue;
        }

        Expense expense{
            id,
            fields[1],
            fields[2],
            fields[3],
            amount
        };

        expenses_.push_back(std::move(expense));
        nextId_ = std::max(nextId_, id + 1);
    }
    return true;
}

bool ExpenseTracker::save() const {
    std::ofstream output(dbFile_, std::ios::trunc);
    if (!output) return false;

    output << std::fixed << std::setprecision(2);
    for (const auto& exp : expenses_) {
        output << exp.id << '\t'
               << escapeField(exp.date) << '\t'
               << escapeField(exp.category) << '\t'
               << escapeField(exp.description) << '\t'
               << exp.amount << '\n';
    }
    return static_cast<bool>(output);
}

Expense& ExpenseTracker::add(Expense expense) {
    expense.id = nextId_++;
    if (expense.date.empty()) {
        expense.date = todayDate();
    }
    expenses_.push_back(std::move(expense));
    return expenses_.back();
}

Expense* ExpenseTracker::find(int id) {
    for (auto& exp : expenses_) {
        if (exp.id == id) return &exp;
    }
    return nullptr;
}

const Expense* ExpenseTracker::find(int id) const {
    for (const auto& exp : expenses_) {
        if (exp.id == id) return &exp;
    }
    return nullptr;
}

bool ExpenseTracker::remove(int id) {
    const auto oldSize = expenses_.size();
    expenses_.erase(std::remove_if(expenses_.begin(), expenses_.end(),
                                   [id](const Expense& e) { return e.id == id; }),
                    expenses_.end());
    return expenses_.size() != oldSize;
}

std::vector<const Expense*> ExpenseTracker::filterByCategory(std::string_view category) const {
    const auto target = lowercase(trim(category));
    std::vector<const Expense*> result;
    for (const auto& exp : expenses_) {
        if (lowercase(exp.category) == target) {
            result.push_back(&exp);
        }
    }
    return result;
}

std::vector<const Expense*> ExpenseTracker::filterByMonth(std::string_view yearMonth) const {
    const auto prefix = trim(yearMonth);
    std::vector<const Expense*> result;
    for (const auto& exp : expenses_) {
        if (exp.date.size() >= 7 && std::string_view(exp.date).substr(0, 7) == prefix) {
            result.push_back(&exp);
        }
    }
    return result;
}

std::vector<const Expense*> ExpenseTracker::search(std::string_view query) const {
    const auto queryLower = lowercase(trim(query));
    std::vector<const Expense*> matches;
    if (queryLower.empty()) return matches;

    for (const auto& exp : expenses_) {
        if (containsIgnoreCase(exp.description, queryLower) ||
            containsIgnoreCase(exp.category, queryLower) ||
            containsIgnoreCase(exp.date, queryLower)) {
            matches.push_back(&exp);
        }
    }
    return matches;
}

ExpenseSummary ExpenseTracker::computeSummary(std::string_view yearMonth) const {
    ExpenseSummary summary;
    const auto monthPrefix = trim(yearMonth);

    std::map<std::string, CategoryTotal> categoryMap;

    for (const auto& exp : expenses_) {
        if (!monthPrefix.empty()) {
            if (exp.date.size() < 7 || std::string_view(exp.date).substr(0, 7) != monthPrefix) {
                continue;
            }
        }

        ++summary.totalCount;
        summary.totalAmount += exp.amount;
        if (exp.amount > summary.highestAmount) {
            summary.highestAmount = exp.amount;
            summary.highestDescription = exp.description + " (" + exp.date + ")";
        }

        const auto key = lowercase(exp.category);
        auto& entry = categoryMap[key];
        if (entry.category.empty()) {
            entry.category = exp.category;
        }
        ++entry.count;
        entry.total += exp.amount;
    }

    if (summary.totalCount > 0) {
        summary.averageAmount = summary.totalAmount / static_cast<double>(summary.totalCount);
    }

    for (auto& [key, val] : categoryMap) {
        summary.byCategory.push_back(std::move(val));
    }

    std::sort(summary.byCategory.begin(), summary.byCategory.end(),
              [](const CategoryTotal& a, const CategoryTotal& b) {
                  if (a.total != b.total) return a.total > b.total;
                  return a.category < b.category;
              });

    return summary;
}

} // namespace exptrack

