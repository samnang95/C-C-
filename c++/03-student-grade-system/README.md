# Student Grade System (C++)

A modular, dependency-free C++17 command-line application for managing student transcripts, course grades, GPA calculation, and class rankings. Data is persisted to `data/students.db`.

## Project Structure

```text
03-student-grade-system/
├── build/
├── data/
├── src/
│   ├── main.cpp
│   ├── grade/
│   │   ├── Student.hpp
│   │   ├── Student.cpp
│   │   ├── GradeBook.hpp
│   │   └── GradeBook.cpp
│   └── ui/
│       ├── ConsoleUI.hpp
│       └── ConsoleUI.cpp
├── tests/
│   └── test_grade.cpp
├── .gitignore
├── compile_flags.txt
├── Makefile
└── README.md
```

## Build, Run & Test

```sh
# Build executable into build/student_grade_system
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
| `add` | Enroll a new student interactively. |
| `list` | List all students with average, GPA, and letter grade. |
| `list top` | List students ranked by highest average. |
| `view <id>` | Display a student's complete course transcript. |
| `grade <id>` | Add or update a course grade (`0`–`100`) for a student. |
| `ungrade <id>` | Remove a course grade from a student. |
| `search <text>` | Search students by name, department, or course title. |
| `stats` | Display overall class average, GPA, and top student. |
| `remove <id>` | Remove a student record. |
| `clear` / `reset` | Clear the terminal screen. |
| `help` | Display command help. |
| `quit` | Save and exit. |

