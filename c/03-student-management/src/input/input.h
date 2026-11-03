#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>

typedef enum {
    INPUT_OK = 0,
    INPUT_RESET,
    INPUT_QUIT,
    INPUT_ERR_NULL
} InputAction;

int parse_int(const char *str, int *out_value);
int parse_double(const char *str, double *out_value);

InputAction read_int_in_range(const char *prompt, int min, int max, int *out_value);
InputAction read_double_in_range(const char *prompt, double min, double max, double *out_value);
InputAction read_nonempty_string(const char *prompt, char *out_buf, size_t buf_size);

#endif /* INPUT_H */

