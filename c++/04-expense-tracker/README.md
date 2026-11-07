# Expense Tracker (C++)

A modular, dependency-free C++17 command-line application for recording personal expenses, filtering by category or month, and generating spending breakdowns. Data is persisted to `data/expenses.db`.

## Project Structure

```text
04-expense-tracker/
├── build/
├── data/
├── src/
│   ├── main.cpp
│   ├── expense/
│   │   ├── Expense.hpp
│   │   ├── Expense.cpp
│   │   ├── ExpenseTracker.hpp
│   │   └── ExpenseTracker.cpp
│   └── ui/
│       ├── ConsoleUI.hpp
│       └── ConsoleUI.cpp
├── tests/
│   └── test_expense.cpp
├── .gitignore
├── compile_flags.txt
├── Makefile
└── README.md
```

## Build, Run & Test

```sh
# Build executable into build/expense_tracker
make

# Run interactively
make run

# Run unit tests
make test

# Clean build artifacts
make clean
```

## Commands

| Command | Description |
| --- | --- |
| `add` | Record a new expense interactively (defaults to today's date). |
| `list` | List all expenses with running total. |
| `list <category>` | List expenses in a specific category. |
| `list <YYYY-MM>` | List expenses for a specific month. |
| `view [id]` | Display full details of an expense. |
| `edit [id]` | Update description, category, amount, or date of an expense. |
| `remove [id]` | Delete an expense record. |
| `search <text>` | Search expenses by description, category, or date. |
| `summary [YYYY-MM]` | Display total spending, average, largest expense, and category breakdown. |
| `clear` / `reset` | Clear the terminal screen. |
| `help` | Display command help. |
| `quit` | Save and exit. |

