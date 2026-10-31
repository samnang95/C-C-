#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "calculator/calculator.h"
#include "input/input.h"
#include "ui/display.h"

static int run_single_expression(int argc, char *argv[]) {
    if (argc != 4 || argv == NULL) {
        fprintf(stderr, "Usage: %s [<number> <op> <number>]\n",
                (argv != NULL && argv[0] != NULL) ? argv[0] : "calculator");
        return EXIT_FAILURE;
    }

    double a = 0.0;
    double b = 0.0;

    if (!parse_number(argv[1], &a) || !parse_number(argv[3], &b) ||
        argv[2] == NULL || strlen(argv[2]) != 1) {
        fprintf(stderr, "Invalid arguments. Example: %s 10 + 5\n", argv[0]);
        return EXIT_FAILURE;
    }

    double result = 0.0;
    CalcStatus status = calc_evaluate(a, argv[2][0], b, &result);
    if (status != CALC_OK) {
        fprintf(stderr, "%s\n", calc_status_message(status));
        return EXIT_FAILURE;
    }

    print_result(result);
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        return run_single_expression(argc, argv);
    }

    print_banner();

    while (1) {
        double a = 0.0;
        double b = 0.0;
        char op = '\0';

        InputAction action = read_number("Enter first number: ", &a);
        if (action == INPUT_QUIT || action == INPUT_ERR_NULL) {
            break;
        }
        if (action == INPUT_RESET) {
            clear_screen();
            print_banner();
            continue;
        }

        action = read_operator(&op);
        if (action == INPUT_QUIT || action == INPUT_ERR_NULL) {
            break;
        }
        if (action == INPUT_RESET) {
            clear_screen();
            print_banner();
            continue;
        }

        action = read_number("Enter second number: ", &b);
        if (action == INPUT_QUIT || action == INPUT_ERR_NULL) {
            break;
        }
        if (action == INPUT_RESET) {
            clear_screen();
            print_banner();
            continue;
        }

        double result = 0.0;
        CalcStatus status = calc_evaluate(a, op, b, &result);
        if (status != CALC_OK) {
            printf("%s\n\n", calc_status_message(status));
            continue;
        }

        print_calculation(a, op, b, result);
    }

    return EXIT_SUCCESS;
}
