#include "input/input.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        /* Drain characters exceeding INPUT_BUFFER_SIZE */
    }
}

int parse_int(const char *str, int *out_value) {
    if (str == NULL || out_value == NULL || *str == '\0') {
        return 0;
    }

    errno = 0;
    char *end = NULL;
    long val = strtol(str, &end, 10);

    if (end == str || errno == ERANGE || val < INT_MIN || val > INT_MAX) {
        return 0;
    }

    while (isspace((unsigned char)*end)) {
        end++;
    }
    if (*end != '\0') {
        return 0;
    }

    *out_value = (int)val;
    return 1;
}

InputAction read_int_in_range(const char *prompt, int min, int max, int *out_value) {
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
            printf("Input too long. Please enter a whole number between %d and %d.\n",
                   min, max);
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

        int value = 0;
        if (!parse_int(p, &value)) {
            printf("Invalid whole number. Enter a number between %d and %d (or 'q' to quit).\n",
                   min, max);
            continue;
        }
        if (value < min || value > max) {
            printf("Out of range. Please enter a number between %d and %d.\n",
                   min, max);
            continue;
        }

        *out_value = value;
        return INPUT_OK;
    }
}

InputAction read_yes_no(const char *prompt, int *out_yes) {
    if (out_yes == NULL) {
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
            printf("Please enter 'y' or 'n'.\n");
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

        if (str_eq_ignore_case(p, "y") || str_eq_ignore_case(p, "yes")) {
            *out_yes = 1;
            return INPUT_OK;
        }
        if (str_eq_ignore_case(p, "n") || str_eq_ignore_case(p, "no")) {
            *out_yes = 0;
            return INPUT_OK;
        }

        printf("Please answer 'y' (yes) or 'n' (no).\n");
    }
}

