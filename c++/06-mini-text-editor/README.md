# Mini Text Editor (C++)

A modular, dependency-free C++17 line-oriented text editor supporting line insertion, editing, deletion, multi-level undo/redo, search & replace, and document statistics. By default it edits `data/notes.txt` (or any file path passed via command-line or `open`).

## Project Structure

```text
06-mini-text-editor/
├── build/
├── data/
├── src/
│   ├── main.cpp
│   ├── editor/
│   │   ├── TextBuffer.hpp
│   │   └── TextBuffer.cpp
│   └── ui/
│       ├── ConsoleUI.hpp
│       └── ConsoleUI.cpp
├── tests/
│   └── test_editor.cpp
├── .gitignore
├── compile_flags.txt
├── Makefile
└── README.md
```

## Build, Run & Test

```sh
# Build executable into build/mini_text_editor
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
| `open <file>` | Open a text file into the buffer. |
| `save [file]` | Save the buffer to the current or specified file path. |
| `print` / `list` | Print all lines in the buffer with line numbers. |
| `append [text]` | Append a single line (or enter multi-line mode ending with `.`). |
| `insert <line> [text]` | Insert a line at 1-based position `<line>`. |
| `edit <line> [text]` | Replace the contents of line `<line>`. |
| `delete <line>` | Delete line `<line>`. |
| `find <text>` | Case-insensitive search across all lines with line & column numbers. |
| `replace` | Find and replace all occurrences of a substring across the buffer. |
| `undo` / `redo` | Undo or redo buffer modifications. |
| `stats` | Display line count, word count, character count, and save status. |
| `new` | Clear all lines in the current buffer. |
| `clear` / `reset` | Clear the terminal screen. |
| `help` | Display command help. |
| `quit` | Prompt to save unsaved changes and exit. |

