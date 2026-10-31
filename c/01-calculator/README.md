# 01 - Calculator

A modular command-line calculator written in standard C11 (`-Wall -Wextra -Wpedantic`), supporting `+`, `-`, `*` (`x`), `/` (`\`), `%`, and `^`.

## Project Structure

```text
src/
├── main.c                          # Entry point & CLI/REPL coordinator
├── calculator/
│   ├── calculator.h                # Public calculator API & CalcStatus enum
│   ├── calculator_basic.c          # calc_add, calc_subtract, calc_multiply
│   ├── calculator_advanced.c       # calc_divide, calc_modulo, calc_power
│   ├── calculator_evaluate.c       # calc_is_valid_op, calc_evaluate
│   └── calculator_status.c         # calc_status_message
├── input/
│   ├── input.h                     # InputAction enum & input/parsing API
│   └── input.c                     # Safe strtod parsing & interactive prompts
└── ui/
    ├── display.h                   # Terminal output & formatting API
    └── display.c                   # Banner, screen clearing, result display

tests/
└── test_calculator.c               # Unit tests for arithmetic, errors & parsing

build/                              # Compiled object files & binaries
Makefile
README.md
.gitignore
```

## Build & Run

```bash
# Build and launch interactive calculator
make run

# Run a single expression from the command line
make
./build/calculator 12.5 + 7.5
./build/calculator 6 '*' 7

# Run unit tests
make test

# Remove compiled artifacts
make clean
```
