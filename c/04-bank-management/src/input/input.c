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

int parse_money_cents(const char *str, int64_t *out_cents) {
    if (str == NULL || out_cents == NULL) {
        return 0;
    }

    const char *p = str;
    while (isspace((unsigned char)*p)) {
        p++;
    }
    if (*p == '$') {
        p++;
    }
    if (!isdigit((unsigned char)*p)) {
        return 0;
    }

    int64_t dollars = 0;
    const int64_t max_dollars = (INT64_MAX - 99) / 100;

    while (isdigit((unsigned char)*p)) {
        int digit = *p - '0';
        if (dollars > (max_dollars - digit) / 10) {
            return 0;
        }
        dollars = dollars * 10 + digit;
        p++;
    }

    int64_t cents = 0;
    if (*p == '.') {
        p++;
        if (!isdigit((unsigned char)*p)) {
            return 0;
        }
        cents = (*p - '0') * 10;
        p++;

        if (isdigit((unsigned char)*p)) {
            cents += (*p - '0');
            p++;
        }
        if (isdigit((unsigned char)*p)) {
            /* Reject more than 2 decimal places (sub-cent amounts) */
            return 0;
        }
    }

    while (isspace((unsigned char)*p)) {
        p++;
    }
    if (*p != '\0') {
        return 0;
    }

    *out_cents = dollars * 100 + cents;
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

InputAction read_money_cents(const char *prompt, int64_t min_cents, int64_t *out_cents) {
    if (out_cents == NULL) {
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
            printf("Input too long. Enter an amount such as 50 or 50.25.\n");
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

        int64_t cents = 0;
        if (!parse_money_cents(p, &cents)) {
            printf("Invalid amount. Enter a dollar amount (e.g. 100 or 25.50).\n");
            continue;
        }
        if (cents < min_cents) {
            printf("Amount must be at least $%lld.%02lld.\n",
                   (long long)(min_cents / 100),
                   (long long)(min_cents % 100));
            continue;
        }

        *out_cents = cents;
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
        if (strlen(p) >= buf_size) {
            printf("Value too long (max %zu characters).\n", buf_size - 1);
            continue;
        }

        snprintf(out_buf, buf_size, "%s", p);
        return INPUT_OK;
    }
}

