#include "game/game.h"

#include <stddef.h>

int game_is_valid_difficulty(int choice) {
    return choice == DIFF_EASY || choice == DIFF_MEDIUM || choice == DIFF_HARD;
}

int game_get_config(Difficulty level, DifficultyConfig *out_config) {
    if (out_config == NULL) {
        return 0;
    }

    switch (level) {
        case DIFF_EASY:
            out_config->level = DIFF_EASY;
            out_config->name = "Easy";
            out_config->min_number = 1;
            out_config->max_number = 50;
            out_config->max_attempts = 10;
            return 1;
        case DIFF_MEDIUM:
            out_config->level = DIFF_MEDIUM;
            out_config->name = "Medium";
            out_config->min_number = 1;
            out_config->max_number = 100;
            out_config->max_attempts = 7;
            return 1;
        case DIFF_HARD:
            out_config->level = DIFF_HARD;
            out_config->name = "Hard";
            out_config->min_number = 1;
            out_config->max_number = 500;
            out_config->max_attempts = 9;
            return 1;
        default:
            return 0;
    }
}

