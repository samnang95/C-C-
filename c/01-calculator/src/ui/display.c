#include "ui/display.h"

#include <stdio.h>

static double clean_zero(double value) {
    return (value == 0.0) ? 0.0 : value;
}

void clear_screen(void) {
    /* Standard ANSI/VT100 escape sequence to clear screen and home cursor */
    printf("\033[2J\033[H");
    fflush(stdout);
}

void print_banner(void) {
    printf("Simple C Calculator\n");
    printf("Commands: 'c' / 'clear' / 'reset' to clear & restart, 'q' / 'quit' to exit.\n\n");
}

void print_result(double value) {
    printf("= %.10g\n", clean_zero(value));
}

void print_calculation(double a, char op, double b, double result) {
    char display_op = (op == '\\') ? '/' : op;
    printf("Result: %.10g %c %.10g = %.10g\n\n",
           clean_zero(a), display_op, clean_zero(b), clean_zero(result));
}

