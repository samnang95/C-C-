#ifndef GAME_H
#define GAME_H

typedef enum {
    DIFF_EASY = 1,
    DIFF_MEDIUM = 2,
    DIFF_HARD = 3
} Difficulty;

typedef struct {
    Difficulty level;
    const char *name;
    int min_number;
    int max_number;
    int max_attempts;
} DifficultyConfig;

typedef enum {
    GAME_IN_PROGRESS = 0,
    GAME_WON,
    GAME_LOST
} GameStatus;

typedef enum {
    GUESS_CORRECT = 0,
    GUESS_TOO_LOW,
    GUESS_TOO_HIGH,
    GUESS_OUT_OF_RANGE,
    GUESS_GAME_OVER,
    GUESS_ERR_NULL
} GuessResult;

typedef struct {
    DifficultyConfig config;
    int secret_number;
    int attempts_used;
    GameStatus status;
} GameState;

/* Difficulty presets (game_config.c) */
int game_is_valid_difficulty(int choice);
int game_get_config(Difficulty level, DifficultyConfig *out_config);

/* Random number generation (game_random.c) */
int game_random_in_range(int min, int max);

/* Core state machine (game_engine.c) */
int game_init(GameState *state, Difficulty level, int secret_number);
int game_start(GameState *state, Difficulty level);
GuessResult game_make_guess(GameState *state, int guess);
int game_remaining_attempts(const GameState *state);

#endif /* GAME_H */

