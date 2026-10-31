#ifndef INPUT_H
#define INPUT_H

typedef enum {
    INPUT_OK = 0,
    INPUT_RESET,
    INPUT_QUIT,
    INPUT_ERR_NULL
} InputAction;

int parse_number(const char *str, double *out_value);
InputAction read_number(const char *prompt, double *out_value);
InputAction read_operator(char *out_op);

#endif /* INPUT_H */

