# 03 - Student Management System

A modular command-line Student Management System written in standard C11 (`-Wall -Wextra -Wpedantic`), practicing dynamic heap arrays (`malloc` / `realloc` / `free`), `memmove` array shifting, and CSV file persistence (`fopen` / `fgets` / `fprintf`).

## Features

- **Core CRUD**: Add, List All, Search (by ID or case-insensitive Name), Update, and Delete student records.
- **Dynamic Heap Storage**: Automatically grows capacity (`realloc`) as records are added and frees all heap memory on exit.
- **CSV File Persistence**: Automatically loads and saves student records to `data/students.csv`.
- **Commands at Any Prompt**: Type `c` / `clear` / `reset` to clear the screen and return to the main menu, or `q` / `quit` to exit cleanly.

## Project Structure

```text
src/
├── main.c                      # Entry point, menu loop & CRUD coordinators
├── student/
│   ├── student.h               # Student, StudentList structs & public API
│   ├── student_list.c          # Dynamic array allocation & CRUD logic
│   └── student_storage.c       # CSV file save/load implementation
├── input/
│   ├── input.h                 # InputAction enum & validated input readers
│   └── input.c                 # Safe int, double & string input handling
└── ui/
    ├── display.h               # Terminal table & menu API
    └── display.c               # Formatted ASCII table & menu output

tests/
└── test_student.c              # Unit tests for dynamic array, CRUD & CSV I/O

data/                           # Default location for students.csv (gitignored)
build/                          # Compiled object files & binaries (gitignored)
Makefile
README.md
.gitignore
```

## Build & Run

```bash
cd c/03-student-management

# Build and start the Student Management System
make run

# Run unit tests
make test

# Remove compiled artifacts
make clean
```

