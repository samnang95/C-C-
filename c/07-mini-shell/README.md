# 07 - Mini Shell

A small POSIX-style shell written in C11. It demonstrates line reading, token parsing, dynamic `argv` allocation, built-ins, process creation, `execvp`, and `waitpid`.

Features:

- External commands resolved through `PATH`
- Quoted (`"..."`, `'...'`) and backslash-escaped arguments
- Built-ins: `cd`, `pwd`, `help`, and `exit`
- Parse and execution-status errors reported to `stderr`

Pipelines, redirection, glob expansion, and command substitution are intentionally outside this project’s scope.

```sh
cd c/07-mini-shell
make run
make test
make clean
```
