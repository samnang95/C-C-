#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "shell/shell.h"

static void test_parser(void) {
    ShellCommand command;
    shell_command_init(&command);

    assert(shell_parse_command("echo hello world\n", &command) == SHELL_OK);
    assert(command.argc == 3 && strcmp(command.argv[0], "echo") == 0);
    assert(strcmp(command.argv[2], "world") == 0);

    assert(shell_parse_command("echo \"hello world\" 'and more'\\!\n", &command) == SHELL_OK);
    assert(command.argc == 3);
    assert(strcmp(command.argv[1], "hello world") == 0);
    assert(strcmp(command.argv[2], "and more!") == 0);

    assert(shell_parse_command("   \t \n", &command) == SHELL_OK && command.argc == 0);
    assert(shell_parse_command("echo 'unfinished\n", &command) == SHELL_ERR_PARSE);
    assert(command.argc == 0);
    assert(shell_parse_command("echo trailing\\\n", &command) == SHELL_ERR_PARSE);

    assert(shell_parse_command(NULL, &command) == SHELL_ERR_NULL);
    shell_command_free(&command);
}

static void test_builtins(void) {
    ShellCommand command;
    shell_command_init(&command);
    assert(shell_parse_command("help", &command) == SHELL_OK);
    assert(shell_is_builtin(&command));
    shell_command_free(&command);
    assert(shell_parse_command("echo", &command) == SHELL_OK);
    assert(!shell_is_builtin(&command));
    shell_command_free(&command);
}

int main(void) {
    test_parser();
    test_builtins();
    puts("All mini-shell unit tests passed!");
    return 0;
}
