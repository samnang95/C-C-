#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shell/shell.h"

#define INPUT_SIZE 1024

int main(void) {
    char line[INPUT_SIZE];
    ShellCommand command;
    shell_command_init(&command);
    puts("Mini Shell. Type 'help' for commands and 'exit' to quit.");

    int should_exit = 0;
    while (!should_exit) {
        fputs("mini-shell> ", stdout);
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL) { putchar('\n'); break; }
        if (strchr(line, '\n') == NULL && !feof(stdin)) {
            int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
            fprintf(stderr, "Input too long.\n"); continue;
        }
        ShellStatus parsed = shell_parse_command(line, &command);
        if (parsed != SHELL_OK) { fprintf(stderr, "Parse error: %s\n", shell_status_message(parsed)); continue; }
        if (command.argc == 0) continue;
        int result = shell_is_builtin(&command) ? shell_run_builtin(&command, &should_exit) : shell_run_external(&command);
        if (result != 0 && !should_exit) fprintf(stderr, "Command exited with status %d\n", result);
    }
    shell_command_free(&command);
    return EXIT_SUCCESS;
}
