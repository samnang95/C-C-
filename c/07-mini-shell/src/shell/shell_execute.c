#include "shell/shell.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int command_is(const ShellCommand *command, const char *name) {
    return command != NULL && command->argc > 0 && strcmp(command->argv[0], name) == 0;
}

int shell_is_builtin(const ShellCommand *command) {
    return command_is(command, "cd") || command_is(command, "pwd") ||
           command_is(command, "help") || command_is(command, "exit");
}

int shell_run_builtin(const ShellCommand *command, int *should_exit) {
    if (command == NULL || should_exit == NULL || command->argc == 0) return 1;
    if (command_is(command, "exit")) {
        if (command->argc > 1) fprintf(stderr, "exit: too many arguments\n");
        else *should_exit = 1;
        return 0;
    }
    if (command_is(command, "help")) {
        puts("Built-ins: cd [directory], pwd, help, exit");
        puts("External commands support quoted and escaped arguments.");
        return 0;
    }
    if (command_is(command, "pwd")) {
        char *directory = getcwd(NULL, 0);
        if (directory == NULL) { perror("pwd"); return 1; }
        puts(directory); free(directory); return 0;
    }
    if (command_is(command, "cd")) {
        if (command->argc > 2) { fprintf(stderr, "cd: too many arguments\n"); return 1; }
        const char *directory = command->argc == 2 ? command->argv[1] : getenv("HOME");
        if (directory == NULL) { fprintf(stderr, "cd: HOME is not set\n"); return 1; }
        if (chdir(directory) != 0) { perror("cd"); return 1; }
        return 0;
    }
    return 1;
}

int shell_run_external(const ShellCommand *command) {
    if (command == NULL || command->argc == 0) return 0;
    pid_t child = fork();
    if (child < 0) { perror("fork"); return 1; }
    if (child == 0) {
        execvp(command->argv[0], command->argv);
        fprintf(stderr, "%s: %s\n", command->argv[0], strerror(errno));
        _exit(errno == ENOENT ? 127 : 126);
    }
    int status = 0;
    if (waitpid(child, &status, 0) < 0) { perror("waitpid"); return 1; }
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) { fprintf(stderr, "Process terminated by signal %d\n", WTERMSIG(status)); return 128 + WTERMSIG(status); }
    return 1;
}
