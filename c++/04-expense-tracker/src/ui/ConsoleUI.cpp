#include "ui/ConsoleUI.hpp"

#include <charconv>
#include <cstdlib>
#include <iomanip>
#include <iostream>

namespace exptrack {

void printHelp() {
    std::cout << "\nCommands:\n"
              << "  add                     Record a new expense\n"
              << "  list [category|YYYY-MM] List all expenses (or filter by category/month)\n"
              << "  view [id]               Show expense details\n"
              << "  edit [id]               Edit an existing expense\n"
              << "  remove [id]             Delete an expense\n"
              << "  search <text>           Search description, category, or date\n"
              << "  summary [YYYY-MM]       Show total spending & category breakdown\n"
              << "  clear / reset           Clear the terminal screen\n"
              << "  help                    Show this help\n"
              << "  quit                    Save and exit\n\n";
}

void printExpense(const Expense& expense) {
    std::cout << std::fixed << std::setprecision(2)
              << "\nExpense #" << expense.id << '\n'
              << "  Date:        " << expense.date << '\n'
              << "  Category:    " << expense.category << '\n'
              << "  Description: " << expense.description << '\n'
              << "  Amount:      $" << expense.amount << "\n\n";
}

void listExpenses(const std::vector<const Expense*>& expenses) {
    if (expenses.empty()) {
        std::cout << "\nNo matching expenses.\n\n";
        return;
    }

    double total = 0.0;
    std::cout << "\n" << std::left << std::setw(5) << "ID"
              << std::setw(13) << "Date"
              << std::setw(18) << "Category"
              << std::setw(32) << "Description"
              << std::right << std::setw(11) << "Amount\n"
              << std::string(79, '-') << '\n';

    std::cout << std::fixed << std::setprecision(2);
    for (const auto* exp : expenses) {
        if (!exp) continue;
        total += exp->amount;
        std::cout << std::left << std::setw(5) << exp->id
                  << std::setw(13) << exp->date
                  << std::setw(18) << exp->category.substr(0, 17)
                  << std::setw(32) << exp->description.substr(0, 31)
                  << std::right << "$" << std::setw(9) << exp->amount << '\n';
    }
    std::cout << std::string(79, '-') << '\n'
              << std::left << std::setw(68) << ("Total (" + std::to_string(expenses.size()) + " items)")
              << std::right << "$" << std::setw(9) << total << "\n\n";
}

void listExpenses(const std::vector<Expense>& expenses) {
    std::vector<const Expense*> ptrs;
    ptrs.reserve(expenses.size());
    for (const auto& exp : expenses) {
        ptrs.push_back(&exp);
    }
    listExpenses(ptrs);
}

void printSummary(const ExpenseSummary& summary, std::string_view monthFilter) {
    std::cout << "\nExpense Summary";
    if (!monthFilter.empty()) {
        std::cout << " (" << monthFilter << ")";
    }
    std::cout << ":\n";

    if (summary.totalCount == 0) {
        std::cout << "  No expenses recorded.\n\n";
        return;
    }

    std::cout << std::fixed << std::setprecision(2)
              << "  Transactions:    " << summary.totalCount << '\n'
              << "  Total spent:     $" << summary.totalAmount << '\n'
              << "  Average expense: $" << summary.averageAmount << '\n'
              << "  Largest expense: $" << summary.highestAmount << " — " << summary.highestDescription << "\n\n"
              << "  By Category:\n"
              << "  " << std::left << std::setw(20) << "Category"
              << std::setw(10) << "Count"
              << std::setw(14) << "Total"
              << "Share\n"
              << "  " << std::string(54, '-') << '\n';

    for (const auto& cat : summary.byCategory) {
        const double share = summary.totalAmount > 0.0
                                 ? (cat.total / summary.totalAmount) * 100.0
                                 : 0.0;
        std::cout << "  " << std::left << std::setw(20) << cat.category.substr(0, 19)
                  << std::setw(10) << cat.count
                  << "$" << std::setw(13) << cat.total
                  << share << "%\n";
    }
    std::cout << '\n';
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

bool parseAmount(std::string_view text, double& amount) {
    std::string trimmed(trim(text));
    if (!trimmed.empty() && trimmed.front() == '$') {
        trimmed = std::string(trim(std::string_view(trimmed).substr(1)));
    }
    if (trimmed.empty()) return false;

    char* endPtr = nullptr;
    const double parsed = std::strtod(trimmed.c_str(), &endPtr);
    if (endPtr == trimmed.c_str() || *endPtr != '\0' || parsed <= 0.0) {
        return false;
    }
    amount = parsed;
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

void addExpenseInteractive(ExpenseTracker& tracker) {
    Expense exp;

    while (std::cin) {
        exp.description = prompt("Description: ");
        if (!std::cin) return;
        if (exp.description.empty()) {
            std::cout << "A description is required.\n";
            continue;
        }
        break;
    }

    while (std::cin) {
        exp.category = prompt("Category (e.g. Food, Transport, Bills): ");
        if (!std::cin) return;
        if (exp.category.empty()) {
            std::cout << "A category is required.\n";
            continue;
        }
        break;
    }

    while (std::cin) {
        const auto raw = prompt("Amount ($): ");
        if (!std::cin) return;
        if (parseAmount(raw, exp.amount)) {
            break;
        }
        std::cout << "Please enter a positive amount.\n";
    }

    const auto defaultDate = todayDate();
    while (std::cin) {
        const auto rawDate = prompt("Date YYYY-MM-DD (" + defaultDate + "): ");
        if (!std::cin) return;
        if (rawDate.empty()) {
            exp.date = defaultDate;
            break;
        }
        if (isValidDate(rawDate)) {
            exp.date = rawDate;
            break;
        }
        std::cout << "Please enter a valid date in YYYY-MM-DD format.\n";
    }

    const auto& added = tracker.add(std::move(exp));
    std::cout << "Added expense " << added.id << ".\n";
}

void editExpenseInteractive(ExpenseTracker& tracker, int id) {
    Expense* exp = tracker.find(id);
    if (!exp) {
        std::cout << "Expense not found.\n";
        return;
    }

    const auto desc = prompt("Description [" + exp->description + "]: ");
    if (!std::cin) return;
    if (!desc.empty()) exp->description = desc;

    const auto cat = prompt("Category [" + exp->category + "]: ");
    if (!std::cin) return;
    if (!cat.empty()) exp->category = cat;

    while (std::cin) {
        const auto rawAmt = prompt("Amount [" + std::to_string(exp->amount) + "]: ");
        if (!std::cin) return;
        if (rawAmt.empty()) break;
        double parsed = 0.0;
        if (parseAmount(rawAmt, parsed)) {
            exp->amount = parsed;
            break;
        }
        std::cout << "Please enter a positive amount.\n";
    }

    while (std::cin) {
        const auto rawDate = prompt("Date YYYY-MM-DD [" + exp->date + "]: ");
        if (!std::cin) return;
        if (rawDate.empty()) break;
        if (isValidDate(rawDate)) {
            exp->date = rawDate;
            break;
        }
        std::cout << "Please enter a valid date in YYYY-MM-DD format.\n";
    }

    std::cout << "Expense " << exp->id << " updated.\n";
}

} // namespace exptrack

