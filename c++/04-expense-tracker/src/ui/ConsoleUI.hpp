#pragma once

#include "expense/Expense.hpp"
#include "expense/ExpenseTracker.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace exptrack {

void printHelp();
void printExpense(const Expense& expense);
void listExpenses(const std::vector<const Expense*>& expenses);
void listExpenses(const std::vector<Expense>& expenses);
void printSummary(const ExpenseSummary& summary, std::string_view monthFilter = "");

bool parseId(std::string_view text, int& id);
bool parseAmount(std::string_view text, double& amount);
std::string prompt(std::string_view label);

void addExpenseInteractive(ExpenseTracker& tracker);
void editExpenseInteractive(ExpenseTracker& tracker, int id);

} // namespace exptrack

