# Library Management (C++)

A modular, dependency-free C++17 command-line application for maintaining a library catalogue. It persists records in `data/books.db`.

## Project Structure

```text
02-library-management/
├── build/
├── data/
├── src/
│   ├── main.cpp
│   ├── library/
│   │   ├── Book.hpp
│   │   ├── Book.cpp
│   │   ├── Library.hpp
│   │   └── Library.cpp
│   └── ui/
│       ├── ConsoleUI.hpp
│       └── ConsoleUI.cpp
├── tests/
│   └── test_library.cpp
├── .gitignore
├── compile_flags.txt
├── Makefile
└── README.md
```

## Build, Run & Test

```sh
# Build executable into build/library_management
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
| `add` | Add a book interactively. |
| `list` | List all books. |
| `list available` | List books that can be borrowed. |
| `list borrowed` | List checked-out books. |
| `view <id>` | Display a book's complete details. |
| `search <text>` | Search title, author, ISBN, category, and borrower. |
| `borrow <id>` | Check out an available book. |
| `return <id>` | Mark a borrowed book as returned. |
| `remove <id>` | Remove a book from the catalogue. |
| `help` | Display command help. |
| `quit` | Save and exit. |
