# 01 - Task Manager (C++)

A modular, dependency-free command-line task manager written in standard C++17 (`-Wall -Wextra -Wpedantic`). Tasks are saved in `data/tasks.db`.

## Project Structure

```text
01-task-manager/
├── build/                  # Compiled object files & binaries (gitignored)
├── data/                   # Database files (tasks.db)
├── src/
│   ├── main.cpp            # Entry point & command loop
│   ├── task/
│   │   ├── Task.hpp        # Task struct & date/priority utilities
│   │   ├── Task.cpp
│   │   ├── TaskManager.hpp # TaskManager CRUD & file persistence
│   │   └── TaskManager.cpp
│   └── ui/
│       ├── ConsoleUI.hpp   # CLI prompts, formatting & help menu
│       └── ConsoleUI.cpp
├── tests/
│   └── test_task.cpp       # Unit tests for task validation, CRUD & DB I/O
├── .gitignore
├── Makefile
└── README.md
```

## Build & Run

```bash
cd c++/01-task-manager

# Build and start the program
make run

# Run unit tests
make test

# Clean build artifacts
make clean
```

## Commands

| Command | Description |
| --- | --- |
| `add` | Create a task interactively. |
| `list` | List all tasks. |
| `list open` | List unfinished tasks. |
| `list done` | List completed tasks. |
| `view <id>` | Show the full details for a task. |
| `done <id>` | Mark a task complete. |
| `reopen <id>` | Mark a task incomplete. |
| `delete <id>` | Permanently remove a task (with confirmation). |
| `search <text>` | Search task titles, descriptions, and tags. |
| `help` | Display commands. |
| `quit` | Save and exit. |
