# 02 - Number Guessing Game

A modular command-line Number Guessing Game written in standard C11 (`-Wall -Wextra -Wpedantic`) with difficulty levels, attempt limits, and safe integer input validation.

## Difficulty Levels

| Level | Range | Max Attempts |
| --- | --- | --- |
| `[1]` Easy | 1 – 50 | 10 |
| `[2]` Medium | 1 – 100 | 7 |
| `[3]` Hard | 1 – 500 | 9 |

## Project Structure

```text
src/
├── main.c                  # Entry point, RNG seeding & game loop coordinator
├── game/
│   ├── game.h              # Difficulty, GameState & public game API
│   ├── game_config.c       # Easy / Medium / Hard presets
│   ├── game_random.c       # Unbiased random integer generation
│   └── game_engine.c       # Round initialization & guess evaluation
├── input/
│   ├── input.h             # InputAction enum & integer/yes-no prompt API
│   └── input.c             # Safe strtol() parsing & command handling
└── ui/
    ├── display.h           # Terminal output & menu API
    └── display.c           # Banner, difficulty menu & guess feedback

tests/
└── test_game.c             # Deterministic unit tests for game engine & input

build/                      # Compiled object files & binaries
Makefile
README.md
.gitignore
```

## Build & Run

```bash
cd c/02-number-guessing

# Build and start the game
make run

# Run unit tests
make test

# Remove compiled artifacts
make clean
```

