# 04 - Bank Management System

A modular command-line Bank Management System written in standard C11 (`-Wall -Wextra -Wpedantic`), practicing fixed-point integer money representation (`int64_t` cents), atomic two-account transfers, and binary file persistence (`fwrite` / `fread`).

## Features

- **Account Management**: Create (Checking or Savings), List All, Search (by Account # or Owner Name), and Close accounts.
- **Transactions**: Deposit, Withdraw (with insufficient-funds protection), and Atomic Transfer between two accounts.
- **Exact Cent Arithmetic (`int64_t`)**: Stores balances in integer cents (`$100.50` = `10050`) to prevent IEEE-754 floating-point rounding drift.
- **Binary File Persistence**: Automatically saves and loads accounts to `data/accounts.dat` with a magic-number & version header.
- **Commands at Any Prompt**: Type `c` / `clear` / `reset` to clear the screen and return to the main menu, or `q` / `quit` to exit cleanly.

## Project Structure

```text
src/
├── main.c                      # Entry point, menu loop & transaction coordinators
├── bank/
│   ├── bank.h                  # BankAccount, BankList structs & public API
│   ├── bank_account.c          # Account CRUD, deposit, withdraw & transfer
│   └── bank_storage.c          # Binary file save/load (fwrite / fread)
├── input/
│   ├── input.h                 # InputAction & int, string, money-cents readers
│   └── input.c                 # Fixed-point money parser & command handling
└── ui/
    ├── display.h               # Money formatting & ASCII table API
    └── display.c               # Formatted currency & account table output

tests/
└── test_bank.c                 # Unit tests for transactions, money parser & binary I/O

data/                           # Default location for accounts.dat (gitignored)
build/                          # Compiled object files & binaries (gitignored)
Makefile
README.md
.gitignore
```

## Build & Run

```bash
cd c/04-bank-management

# Build and launch the Bank Management System
make run

# Run unit tests
make test

# Remove compiled artifacts
make clean
```

