#ifndef MINI_SHELL_H
#define MINI_SHELL_H

#include <stddef.h>

typedef enum {
    SHELL_OK = 0,
    SHELL_ERR_NULL,
    SHELL_ERR_NOMEM,
    SHELL_ERR_PARSE,
    SHELL_ERR_TOO_MANY_ARGS
} ShellStatus;

typedef struct {
    char **argv;
    size_t argc;
} ShellCommand;

void shell_command_init(ShellCommand *command);
void shell_command_free(ShellCommand *command);
ShellStatus shell_parse_command(const char *line, ShellCommand *out_command);
const char *shell_status_message(ShellStatus status);

int shell_is_builtin(const ShellCommand *command);
int shell_run_builtin(const ShellCommand *command, int *should_exit);
int shell_run_external(const ShellCommand *command);

#endif /* MINI_SHELL_H */
