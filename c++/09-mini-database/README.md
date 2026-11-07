# Mini Relational Database (C++)

A modular, dependency-free C++17 relational database engine and interactive query shell. Supports creating/dropping tables with custom schemas, auto-incrementing primary keys (`id`), inserting rows, querying with `column=value` filters, updating columns, deleting rows, and persisting schemas & records to `data/minidb.db`.

## Project Structure

```text
09-mini-database/
├── build/
├── data/
├── src/
│   ├── main.cpp
│   ├── db/
│   │   ├── Table.hpp
│   │   ├── Table.cpp
│   │   ├── Database.hpp
│   │   └── Database.cpp
│   └── ui/
│       ├── ConsoleUI.hpp
│       └── ConsoleUI.cpp
├── tests/
│   └── test_db.cpp
├── .gitignore
├── compile_flags.txt
├── Makefile
└── README.md
```

## Build, Run & Test

```sh
# Build executable into build/mini_database
make

# Run interactive database shell
make run

# Run unit tests
make test

# Clean build artifacts
make clean
```

## Commands

| Command | Description |
| --- | --- |
| `tables` / `list` | List all tables, row counts, and column schemas. |
| `create <table> <col1,col2,...>` | Create a new table with the given columns. |
| `describe <table>` | Display column definitions for a table. |
| `drop <table>` | Drop a table and all its rows. |
| `insert [table] [val1,val2,...]` | Insert a row into a table (prompts per column if omitted). |
| `select <table> [col=value]` | Select all rows or filter rows matching `col=value`. |
| `update <table> <id> <col=value>` | Update a column value for row `<id>`. |
| `delete <table> <id>` | Delete row `<id>` from a table. |
| `clear` / `reset` | Clear the terminal screen. |
| `help` | Display command help. |
| `quit` | Save database and exit. |

