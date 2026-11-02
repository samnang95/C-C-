#include "game/game.h"

#include <stddef.h>

int game_init(GameState *state, Difficulty level, int secret_number) {
    if (state == NULL) {
        return 0;
    }

    DifficultyConfig cfg;
    if (!game_get_config(level, &cfg)) {
        return 0;
    }
    if (secret_number < cfg.min_number || secret_number > cfg.max_number) {
        return 0;
    }

    state->config = cfg;
    state->secret_number = secret_number;
    state->attempts_used = 0;
    state->status = GAME_IN_PROGRESS;
    return 1;
}

int game_start(GameState *state, Difficulty level) {
    if (state == NULL) {
        return 0;
    }

    DifficultyConfig cfg;
    if (!game_get_config(level, &cfg)) {
        return 0;
    }

    int secret = game_random_in_range(cfg.min_number, cfg.max_number);
    return game_init(state, level, secret);
}

GuessResult game_make_guess(GameState *state, int guess) {
    if (state == NULL) {
        return GUESS_ERR_NULL;
    }
    if (state->status != GAME_IN_PROGRESS) {
        return GUESS_GAME_OVER;
    }
    if (guess < state->config.min_number || guess > state->config.max_number) {
        return GUESS_OUT_OF_RANGE;
    }

    state->attempts_used++;

    if (guess == state->secret_number) {
        state->status = GAME_WON;
        return GUESS_CORRECT;
    }

    if (state->attempts_used >= state->config.max_attempts) {
        state->status = GAME_LOST;
    }

    return (guess < state->secret_number) ? GUESS_TOO_LOW : GUESS_TOO_HIGH;
}

int game_remaining_attempts(const GameState *state) {
    if (state == NULL || state->attempts_used >= state->config.max_attempts) {
        return 0;
    }
    return state->config.max_attempts - state->attempts_used;
}

