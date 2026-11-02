#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game/game.h"
#include "input/input.h"
#include "ui/display.h"

static InputAction play_round(GameState *state) {
    if (state == NULL) {
        return INPUT_ERR_NULL;
    }

    print_round_intro(state);

    while (state->status == GAME_IN_PROGRESS) {
        char prompt[128];
        snprintf(prompt, sizeof(prompt),
                 "[Attempt %d/%d] Enter guess (%d-%d): ",
                 state->attempts_used + 1,
                 state->config.max_attempts,
                 state->config.min_number,
                 state->config.max_number);

        int guess = 0;
        InputAction action = read_int_in_range(prompt,
                                               state->config.min_number,
                                               state->config.max_number,
                                               &guess);
        if (action != INPUT_OK) {
            return action;
        }

        GuessResult result = game_make_guess(state, guess);
        print_guess_feedback(state, result);
    }

    return INPUT_OK;
}

int main(void) {
    /* Seed the pseudo-random number generator once at program startup */
    srand((unsigned int)time(NULL));

    print_banner();

    while (1) {
        print_difficulty_menu();

        int choice = 0;
        InputAction action = read_int_in_range("Choose difficulty (1-3): ",
                                               DIFF_EASY, DIFF_HARD, &choice);
        if (action == INPUT_QUIT || action == INPUT_ERR_NULL) {
            break;
        }
        if (action == INPUT_RESET) {
            clear_screen();
            print_banner();
            continue;
        }

        GameState state;
        if (!game_start(&state, (Difficulty)choice)) {
            fprintf(stderr, "Failed to initialize game round.\n");
            return EXIT_FAILURE;
        }

        action = play_round(&state);
        if (action == INPUT_QUIT || action == INPUT_ERR_NULL) {
            break;
        }
        if (action == INPUT_RESET) {
            clear_screen();
            print_banner();
            continue;
        }

        int play_again = 0;
        action = read_yes_no("Play again? (y/n): ", &play_again);
        if (action == INPUT_RESET) {
            clear_screen();
            print_banner();
            continue;
        }
        if (action != INPUT_OK || !play_again) {
            break;
        }

        printf("\n");
    }

    printf("Thanks for playing!\n");
    return EXIT_SUCCESS;
}

