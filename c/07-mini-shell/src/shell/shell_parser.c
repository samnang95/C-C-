#include "shell/shell.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_ARGS 8

static ShellStatus add_arg(ShellCommand *command, char *argument, size_t *capacity) {
    if (command->argc + 1 >= *capacity) {
        size_t new_capacity = *capacity == 0 ? INITIAL_ARGS : *capacity * 2;
        char **new_args = realloc(command->argv, new_capacity * sizeof(*new_args));
        if (new_args == NULL) return SHELL_ERR_NOMEM;
        command->argv = new_args;
        *capacity = new_capacity;
    }
    command->argv[command->argc++] = argument;
    command->argv[command->argc] = NULL;
    return SHELL_OK;
}

void shell_command_init(ShellCommand *command) {
    if (command == NULL) return;
    command->argv = NULL;
    command->argc = 0;
}

void shell_command_free(ShellCommand *command) {
    if (command == NULL) return;
    for (size_t i = 0; i < command->argc; ++i) free(command->argv[i]);
    free(command->argv);
    shell_command_init(command);
}

ShellStatus shell_parse_command(const char *line, ShellCommand *out_command) {
    if (line == NULL || out_command == NULL) return SHELL_ERR_NULL;
    shell_command_free(out_command);
    size_t capacity = 0;
    const char *cursor = line;

    while (1) {
        while (isspace((unsigned char)*cursor)) ++cursor;
        if (*cursor == '\0' || *cursor == '\n') return SHELL_OK;

        size_t max_length = strlen(cursor) + 1;
        char *argument = malloc(max_length);
        if (argument == NULL) { shell_command_free(out_command); return SHELL_ERR_NOMEM; }
        size_t length = 0;
        char quote = '\0';

        while (*cursor != '\0' && *cursor != '\n') {
            if (quote == '\0' && isspace((unsigned char)*cursor)) break;
            if (*cursor == '\\') {
                ++cursor;
                if (*cursor == '\0' || *cursor == '\n') { free(argument); shell_command_free(out_command); return SHELL_ERR_PARSE; }
                argument[length++] = *cursor++;
            } else if (*cursor == '\'' || *cursor == '"') {
                if (quote == '\0') quote = *cursor++;
                else if (quote == *cursor) { quote = '\0'; ++cursor; }
                else argument[length++] = *cursor++;
            } else {
                argument[length++] = *cursor++;
            }
        }
        if (quote != '\0') { free(argument); shell_command_free(out_command); return SHELL_ERR_PARSE; }
        argument[length] = '\0';
        ShellStatus status = add_arg(out_command, argument, &capacity);
        if (status != SHELL_OK) { free(argument); shell_command_free(out_command); return status; }
    }
}

const char *shell_status_message(ShellStatus status) {
    switch (status) {
        case SHELL_OK: return "OK";
        case SHELL_ERR_NULL: return "internal null pointer";
        case SHELL_ERR_NOMEM: return "out of memory";
        case SHELL_ERR_PARSE: return "unclosed quote or trailing escape";
        case SHELL_ERR_TOO_MANY_ARGS: return "too many arguments";
        default: return "unknown shell error";
    }
}
