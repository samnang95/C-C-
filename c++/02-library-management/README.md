# Library Management (C++)

A dependency-free C++17 command-line application for maintaining a small library catalogue. It stores its data in `books.db` in the directory where it runs.

## Run

From this directory:

```sh
make run
```

Or build with CMake:

```sh
cmake -S . -B build
cmake --build build
./build/library_management
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

ISBN and category are optional. A borrower name is required when borrowing a book. You can enter `clear` or `reset` only after quitting the program and returning to your regular terminal prompt.
