#include "input/input.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
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
        /* Drain excess characters beyond INPUT_BUFFER_SIZE */
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

int parse_double(const char *str, double *out_value) {
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
            printf("Input too long. Enter a whole number between %d and %d.\n", min, max);
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
            printf("Invalid whole number. Enter an integer between %d and %d.\n", min, max);
            continue;
        }
        if (value < min || value > max) {
            printf("Out of range. Enter a number between %d and %d.\n", min, max);
            continue;
        }

        *out_value = value;
        return INPUT_OK;
    }
}

InputAction read_double_in_range(const char *prompt, double min, double max, double *out_value) {
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
            printf("Input too long. Enter a number between %.2f and %.2f.\n", min, max);
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

        double value = 0.0;
        if (!parse_double(p, &value)) {
            printf("Invalid number. Enter a value between %.2f and %.2f.\n", min, max);
            continue;
        }
        if (value < min || value > max) {
            printf("Out of range. Enter a value between %.2f and %.2f.\n", min, max);
            continue;
        }

        *out_value = value;
        return INPUT_OK;
    }
}

InputAction read_nonempty_string(const char *prompt, char *out_buf, size_t buf_size) {
    if (out_buf == NULL || buf_size < 2) {
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
            printf("Input too long (max %zu characters).\n", buf_size - 1);
            continue;
        }

        char *p = trim_whitespace(line);
        if (*p == '\0') {
            printf("Value cannot be empty.\n");
            continue;
        }
        if (is_quit_cmd(p)) {
            return INPUT_QUIT;
        }
        if (is_reset_cmd(p)) {
            return INPUT_RESET;
        }
        if (strchr(p, ',') != NULL) {
            printf("Commas (',') are not allowed in this field.\n");
            continue;
        }
        if (strlen(p) >= buf_size) {
            printf("Value too long (max %zu characters).\n", buf_size - 1);
            continue;
        }

        snprintf(out_buf, buf_size, "%s", p);
        return INPUT_OK;
    }
}

