#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "game/game.h"
#include "input/input.h"

static void test_difficulty_configs(void) {
    DifficultyConfig cfg;

    assert(game_is_valid_difficulty(DIFF_EASY));
    assert(game_is_valid_difficulty(DIFF_MEDIUM));
    assert(game_is_valid_difficulty(DIFF_HARD));
    assert(!game_is_valid_difficulty(0));
    assert(!game_is_valid_difficulty(4));

    assert(game_get_config(DIFF_EASY, &cfg) == 1);
    assert(cfg.min_number == 1 && cfg.max_number == 50 && cfg.max_attempts == 10);

    assert(game_get_config(DIFF_MEDIUM, &cfg) == 1);
    assert(cfg.min_number == 1 && cfg.max_number == 100 && cfg.max_attempts == 7);

    assert(game_get_config(DIFF_HARD, &cfg) == 1);
    assert(cfg.min_number == 1 && cfg.max_number == 500 && cfg.max_attempts == 9);

    assert(game_get_config(DIFF_EASY, NULL) == 0);
    assert(game_get_config((Difficulty)99, &cfg) == 0);
}

static void test_winning_round(void) {
    GameState state;

    assert(game_init(&state, DIFF_MEDIUM, 42) == 1);
    assert(state.status == GAME_IN_PROGRESS);
    assert(game_remaining_attempts(&state) == 7);

    /* Out-of-range guesses must not consume an attempt */
    assert(game_make_guess(&state, 0) == GUESS_OUT_OF_RANGE);
    assert(game_make_guess(&state, 101) == GUESS_OUT_OF_RANGE);
    assert(state.attempts_used == 0);

    assert(game_make_guess(&state, 20) == GUESS_TOO_LOW);
    assert(game_remaining_attempts(&state) == 6);

    assert(game_make_guess(&state, 80) == GUESS_TOO_HIGH);
    assert(game_remaining_attempts(&state) == 5);

    assert(game_make_guess(&state, 42) == GUESS_CORRECT);
    assert(state.status == GAME_WON);
    assert(state.attempts_used == 3);

    /* Further guesses after winning return GUESS_GAME_OVER */
    assert(game_make_guess(&state, 42) == GUESS_GAME_OVER);
}

static void test_losing_round_and_null_guards(void) {
    GameState state;

    assert(game_init(NULL, DIFF_EASY, 10) == 0);
    assert(game_init(&state, DIFF_EASY, 999) == 0);
    assert(game_make_guess(NULL, 10) == GUESS_ERR_NULL);
    assert(game_remaining_attempts(NULL) == 0);

    assert(game_init(&state, DIFF_MEDIUM, 77) == 1);
    for (int i = 0; i < 6; i++) {
        assert(game_make_guess(&state, 10) == GUESS_TOO_LOW);
        assert(state.status == GAME_IN_PROGRESS);
    }

    /* 7th missed guess ends the game as GAME_LOST */
    assert(game_make_guess(&state, 90) == GUESS_TOO_HIGH);
    assert(state.status == GAME_LOST);
    assert(game_remaining_attempts(&state) == 0);
    assert(game_make_guess(&state, 77) == GUESS_GAME_OVER);
}

static void test_random_bounds(void) {
    srand(12345U);

    for (int i = 0; i < 500; i++) {
        int r = game_random_in_range(1, 50);
        assert(r >= 1 && r <= 50);
    }

    assert(game_random_in_range(7, 7) == 7);

    /* Reversed bounds are handled defensively */
    int swapped = game_random_in_range(20, 10);
    assert(swapped >= 10 && swapped <= 20);
}

static void test_int_parsing(void) {
    int val = 0;

    assert(parse_int("42", &val) == 1 && val == 42);
    assert(parse_int("  -15  ", &val) == 1 && val == -15);
    assert(parse_int("010", &val) == 1 && val == 10); /* Base-10, not octal 8 */

    assert(parse_int(NULL, &val) == 0);
    assert(parse_int("10", NULL) == 0);
    assert(parse_int("", &val) == 0);
    assert(parse_int("   ", &val) == 0);
    assert(parse_int("12.5", &val) == 0);
    assert(parse_int("12abc", &val) == 0);
    assert(parse_int("99999999999999999999", &val) == 0);
    assert(parse_int("2147483648", &val) == 0);  /* INT_MAX + 1 on 32-bit int */
    assert(parse_int("-2147483649", &val) == 0); /* INT_MIN - 1 on 32-bit int */

    assert(read_int_in_range("Prompt: ", 1, 10, NULL) == INPUT_ERR_NULL);
    assert(read_yes_no("Prompt: ", NULL) == INPUT_ERR_NULL);
}

int main(void) {
    test_difficulty_configs();
    test_winning_round();
    test_losing_round_and_null_guards();
    test_random_bounds();
    test_int_parsing();

    printf("All number-guessing unit tests passed!\n");
    return 0;
}

