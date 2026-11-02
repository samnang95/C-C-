#ifndef UI_DISPLAY_H
#define UI_DISPLAY_H

#include "game/game.h"

void clear_screen(void);
void print_banner(void);
void print_difficulty_menu(void);
void print_round_intro(const GameState *state);
void print_guess_feedback(const GameState *state, GuessResult result);

#endif /* UI_DISPLAY_H */

