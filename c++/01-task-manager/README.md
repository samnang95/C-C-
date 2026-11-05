# Task Manager (C++)

A small, dependency-free command-line task manager written in C++17. Tasks are saved in `tasks.db` in the directory where you run the program.

## Build

```sh
cmake -S . -B build
cmake --build build
./build/task_manager
```

If CMake is not installed, build directly with `make` (or compile `src/main.cpp` with any C++17 compiler).

On Windows the executable is normally `build\\Debug\\task_manager.exe` or `build\\task_manager.exe`.

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

Dates use `YYYY-MM-DD`; leave the due-date prompt blank if it has no deadline. Valid priorities are `low`, `medium`, and `high`.
