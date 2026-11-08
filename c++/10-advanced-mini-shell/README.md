# Advanced Mini Shell (C++)

A modular, dependency-free C++17 POSIX command-line shell supporting multi-stage pipelines (`|`), input/output redirection (`<`, `>`, `>>`), background job execution (`&`), environment variable expansion (`$VAR`, `${VAR}`, `$?`), aliases (`alias`, `unalias`), history expansion (`!!`, `!N`), and persistent command history in `data/shell_history.db`.

## Project Structure

```text
10-advanced-mini-shell/
├── build/
├── data/
├── src/
│   ├── main.cpp
│   ├── shell/
│   │   ├── CommandParser.hpp
│   │   ├── CommandParser.cpp
│   │   ├── ShellEngine.hpp
│   │   └── ShellEngine.cpp
│   └── ui/
│       ├── ConsoleUI.hpp
│       └── ConsoleUI.cpp
├── tests/
│   └── test_shell.cpp
├── .gitignore
├── compile_flags.txt
├── Makefile
└── README.md
```

## Build, Run & Test

```sh
# Build executable into build/advanced_mini_shell
make

# Run interactive shell
make run

# Run unit tests
make test

# Clean build artifacts
make clean
```

## Built-in Commands & Operators

| Command / Operator | Description |
| --- | --- |
| `cd [dir]` | Change current working directory (defaults to `$HOME`). |
| `pwd` | Print current working directory. |
| `export KEY=VALUE` | Set an environment variable. |
| `unset KEY` | Unset an environment variable. |
| `alias [name=cmd]` | List all aliases or define a new alias. |
| `unalias <name>` | Remove an alias. |
| `history` / `!!` / `!N` | View command history or repeat previous/Nth command. |
| `jobs` | List background jobs started with `&`. |
| `cmd1 \| cmd2` | Pipe standard output of `cmd1` into `cmd2`. |
| `cmd > file` / `cmd >> file` | Redirect standard output to a file (overwrite or append). |
| `cmd < file` | Redirect standard input from a file. |
| `cmd &` | Execute command in the background. |
| `clear` / `reset` | Clear the terminal screen. |
| `help` | Display built-in help. |
| `exit` / `quit` | Save command history and exit. |

