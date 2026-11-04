#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>

typedef enum { INPUT_OK = 0, INPUT_RESET, INPUT_QUIT, INPUT_ERR_NULL } InputAction;

int parse_int(const char *text, int *out_value);
InputAction read_int_in_range(const char *prompt, int min, int max, int *out_value);
InputAction read_nonempty_string(const char *prompt, char *out_buffer, size_t buffer_size);

#endif /* INPUT_H */
