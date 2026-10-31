#include "input/input.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "calculator/calculator.h"

#define INPUT_BUFFER_SIZE 256

static char *trim_whitespace(char *str) {
    if (str == NULL) {
        return NULL;
    }

    while (isspace((unsigned char)*str)) {
        str++;
    }
    if (*str == '\0') {
        return str;
    }

    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        *end-- = '\0';
    }
    return str;
}

static int str_eq_ignore_case(const char *a, const char *b) {
    if (a == NULL || b == NULL) {
        return 0;
    }

    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

static int is_quit_cmd(const char *s) {
    return str_eq_ignore_case(s, "q") ||
           str_eq_ignore_case(s, "quit") ||
           str_eq_ignore_case(s, "exit");
}

static int is_reset_cmd(const char *s) {
    return str_eq_ignore_case(s, "c") ||
           str_eq_ignore_case(s, "clear") ||
           str_eq_ignore_case(s, "clean") ||
           str_eq_ignore_case(s, "reset");
}

static void discard_remaining_line(void) {
    int ch = 0;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        /* Drain excess characters beyond INPUT_BUFFER_SIZE */
    }
}

int parse_number(const char *str, double *out_value) {
    if (str == NULL || out_value == NULL || *str == '\0') {
        return 0;
    }

    errno = 0;
    char *end = NULL;
    double val = strtod(str, &end);

    if (end == str || errno == ERANGE || !isfinite(val)) {
        return 0;
    }

    while (isspace((unsigned char)*end)) {
        end++;
    }
    if (*end != '\0') {
        return 0;
    }

    *out_value = val;
    return 1;
}

InputAction read_number(const char *prompt, double *out_value) {
    if (out_value == NULL) {
        return INPUT_ERR_NULL;
    }

    char line[INPUT_BUFFER_SIZE];
    const char *label = (prompt != NULL) ? prompt : "";

    while (1) {
        printf("%s", label);
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            return INPUT_QUIT;
        }

        if (strchr(line, '\n') == NULL && !feof(stdin)) {
            discard_remaining_line();
            printf("Input too long. Please enter a shorter number.\n");
            continue;
        }

        char *p = trim_whitespace(line);
        if (*p == '\0') {
            continue;
        }
        if (is_quit_cmd(p)) {
            return INPUT_QUIT;
        }
        if (is_reset_cmd(p)) {
            return INPUT_RESET;
        }

        if (!parse_number(p, out_value)) {
            printf("Invalid or out-of-range number. Please try again (or 'q' to quit).\n");
            continue;
        }

        return INPUT_OK;
    }
}

InputAction read_operator(char *out_op) {
    if (out_op == NULL) {
        return INPUT_ERR_NULL;
    }

    char line[INPUT_BUFFER_SIZE];

    printf("Options: [ - ]  [ + ]  [ x ]  [ / ]  [ %% ]  [ ^ ]\n");

    while (1) {
        printf("Choose operator: ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            return INPUT_QUIT;
        }

        if (strchr(line, '\n') == NULL && !feof(stdin)) {
            discard_remaining_line();
            printf("Input too long. Choose one of: -  +  x  /  %%  ^\n");
            continue;
        }

        char *p = trim_whitespace(line);
        if (*p == '\0') {
            continue;
        }
        if (is_quit_cmd(p)) {
            return INPUT_QUIT;
        }
        if (is_reset_cmd(p)) {
            return INPUT_RESET;
        }

        if (strlen(p) == 1 && calc_is_valid_op(p[0])) {
            *out_op = p[0];
            return INPUT_OK;
        }

        printf("Invalid operator. Choose one of: -  +  x  /  %%  ^\n");
    }
}

