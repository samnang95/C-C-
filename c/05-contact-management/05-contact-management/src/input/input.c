#include "input/input.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_BUFFER_SIZE 256

static char *trim(char *text) {
    while (isspace((unsigned char)*text)) ++text;
    char *end = text + strlen(text);
    while (end > text && isspace((unsigned char)end[-1])) *--end = '\0';
    return text;
}

static int equals_ignore_case(const char *a, const char *b) {
    while (*a && *b && tolower((unsigned char)*a) == tolower((unsigned char)*b)) { ++a; ++b; }
    return *a == '\0' && *b == '\0';
}

static InputAction command_action(const char *text) {
    if (equals_ignore_case(text, "q") || equals_ignore_case(text, "quit") || equals_ignore_case(text, "exit"))
        return INPUT_QUIT;
    if (equals_ignore_case(text, "c") || equals_ignore_case(text, "clear") || equals_ignore_case(text, "reset"))
        return INPUT_RESET;
    return INPUT_OK;
}

static int read_line(char line[INPUT_BUFFER_SIZE]) {
    if (fgets(line, INPUT_BUFFER_SIZE, stdin) == NULL) return 0;
    if (strchr(line, '\n') != NULL || feof(stdin)) return 1;
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) { }
    return -1;
}

int parse_int(const char *text, int *out_value) {
    if (text == NULL || out_value == NULL || *text == '\0') return 0;
    errno = 0;
    char *end = NULL;
    long value = strtol(text, &end, 10);
    while (isspace((unsigned char)*end)) ++end;
    if (end == text || *end != '\0' || errno == ERANGE || value < INT_MIN || value > INT_MAX) return 0;
    *out_value = (int)value;
    return 1;
}

InputAction read_int_in_range(const char *prompt, int min, int max, int *out_value) {
    if (out_value == NULL) return INPUT_ERR_NULL;
    char line[INPUT_BUFFER_SIZE];
    for (;;) {
        printf("%s", prompt == NULL ? "" : prompt);
        fflush(stdout);
        int result = read_line(line);
        if (result == 0) { printf("\n"); return INPUT_QUIT; }
        if (result < 0) { printf("Input too long.\n"); continue; }
        char *value = trim(line);
        InputAction action = command_action(value);
        if (action != INPUT_OK) return action;
        int parsed;
        if (!parse_int(value, &parsed) || parsed < min || parsed > max) {
            printf("Enter a whole number between %d and %d.\n", min, max);
            continue;
        }
        *out_value = parsed;
        return INPUT_OK;
    }
}

InputAction read_nonempty_string(const char *prompt, char *out_buffer, size_t buffer_size) {
    if (out_buffer == NULL || buffer_size < 2) return INPUT_ERR_NULL;
    char line[INPUT_BUFFER_SIZE];
    for (;;) {
        printf("%s", prompt == NULL ? "" : prompt);
        fflush(stdout);
        int result = read_line(line);
        if (result == 0) { printf("\n"); return INPUT_QUIT; }
        if (result < 0) { printf("Input too long (max %zu characters).\n", buffer_size - 1); continue; }
        char *value = trim(line);
        InputAction action = command_action(value);
        if (action != INPUT_OK) return action;
        if (*value == '\0') { printf("Value cannot be empty.\n"); continue; }
        if (strchr(value, ',') != NULL) { printf("Commas are not allowed in this field.\n"); continue; }
        if (strlen(value) >= buffer_size) { printf("Input too long (max %zu characters).\n", buffer_size - 1); continue; }
        snprintf(out_buffer, buffer_size, "%s", value);
        return INPUT_OK;
    }
}
