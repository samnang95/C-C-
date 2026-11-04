#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>
#include <stdint.h>

typedef enum { INPUT_OK = 0, INPUT_RESET, INPUT_QUIT, INPUT_ERR_NULL } InputAction;
int parse_int(const char *text, int *out_value);
int parse_money_cents(const char *text, int64_t *out_cents);
InputAction read_int_in_range(const char *prompt, int min, int max, int *out_value);
InputAction read_money(const char *prompt, int64_t *out_cents);
InputAction read_nonempty_string(const char *prompt, char *out_buffer, size_t buffer_size);
#endif
