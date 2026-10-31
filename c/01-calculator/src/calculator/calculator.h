#ifndef CALCULATOR_H
#define CALCULATOR_H

typedef enum {
    CALC_OK = 0,
    CALC_ERR_DIV_BY_ZERO,
    CALC_ERR_INVALID_OP,
    CALC_ERR_NULL_PTR,
    CALC_ERR_MATH_RANGE
} CalcStatus;

/* Basic operations (calculator_basic.c) */
double calc_add(double a, double b);
double calc_subtract(double a, double b);
double calc_multiply(double a, double b);

/* Advanced operations (calculator_advanced.c) */
CalcStatus calc_divide(double a, double b, double *result);
CalcStatus calc_modulo(double a, double b, double *result);
double calc_power(double a, double b);

/* Dispatcher & status messages */
int calc_is_valid_op(char op);
CalcStatus calc_evaluate(double a, char op, double b, double *result);
const char *calc_status_message(CalcStatus status);

#endif /* CALCULATOR_H */
