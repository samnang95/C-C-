#include "ui/display.h"

#include <stdio.h>

void clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void print_banner(void) {
    printf("========================================\n");
    printf("          Number Guessing Game\n");
    printf("========================================\n");
    printf("Commands: 'c' / 'clear' to restart round, 'q' / 'quit' to exit.\n\n");
}

void print_difficulty_menu(void) {
    static const Difficulty levels[] = {DIFF_EASY, DIFF_MEDIUM, DIFF_HARD};
    size_t count = sizeof(levels) / sizeof(levels[0]);

    printf("Select Difficulty:\n");
    for (size_t i = 0; i < count; i++) {
        DifficultyConfig cfg;
        if (game_get_config(levels[i], &cfg)) {
            printf("  [%d] %-6s (%d - %-3d | %2d attempts)\n",
                   (int)cfg.level, cfg.name,
                   cfg.min_number, cfg.max_number, cfg.max_attempts);
        }
    }
}

void print_round_intro(const GameState *state) {
    if (state == NULL) {
        return;
    }
    printf("\n[%s Mode] I picked a secret number between %d and %d. You have %d attempts!\n\n",
           state->config.name,
           state->config.min_number,
           state->config.max_number,
           state->config.max_attempts);
}

void print_guess_feedback(const GameState *state, GuessResult result) {
    if (state == NULL) {
        return;
    }

    int left = game_remaining_attempts(state);

    switch (result) {
        case GUESS_CORRECT:
            printf("*** Correct! You guessed %d in %d %s! ***\n\n",
                   state->secret_number,
                   state->attempts_used,
                   (state->attempts_used == 1) ? "attempt" : "attempts");
            return;
        case GUESS_TOO_LOW:
            printf("-> Too low!");
            break;
        case GUESS_TOO_HIGH:
            printf("-> Too high!");
            break;
        case GUESS_OUT_OF_RANGE:
            printf("-> Out of range! Guess between %d and %d.\n\n",
                   state->config.min_number, state->config.max_number);
            return;
        case GUESS_GAME_OVER:
        case GUESS_ERR_NULL:
        default:
            return;
    }

    if (state->status == GAME_LOST) {
        printf("\nGame over! You used all %d attempts. The secret number was %d.\n\n",
               state->config.max_attempts, state->secret_number);
    } else {
        printf(" (%d %s left)\n\n",
               left, (left == 1) ? "attempt" : "attempts");
    }
}

