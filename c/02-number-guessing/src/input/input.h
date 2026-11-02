#ifndef INPUT_H
#define INPUT_H

typedef enum {
    INPUT_OK = 0,
    INPUT_RESET,
    INPUT_QUIT,
    INPUT_ERR_NULL
} InputAction;

int parse_int(const char *str, int *out_value);
InputAction read_int_in_range(const char *prompt, int min, int max, int *out_value);
InputAction read_yes_no(const char *prompt, int *out_yes);

#endif /* INPUT_H */

