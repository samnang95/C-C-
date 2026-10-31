#include "calculator/calculator.h"

#include <math.h>
#include <stddef.h>

int calc_is_valid_op(char op) {
    switch (op) {
        case '+':
        case '-':
        case '*':
        case 'x':
        case 'X':
        case '/':
        case '\\':
        case '%':
        case '^':
            return 1;
        default:
            return 0;
    }
}

CalcStatus calc_evaluate(double a, char op, double b, double *result) {
    if (result == NULL) {
        return CALC_ERR_NULL_PTR;
    }
    if (!isfinite(a) || !isfinite(b)) {
        return CALC_ERR_MATH_RANGE;
    }

    double val = 0.0;
    CalcStatus status = CALC_OK;

    switch (op) {
        case '+':
            val = calc_add(a, b);
            break;
        case '-':
            val = calc_subtract(a, b);
            break;
        case '*':
        case 'x':
        case 'X':
            val = calc_multiply(a, b);
            break;
        case '/':
        case '\\':
            status = calc_divide(a, b, &val);
            break;
        case '%':
            status = calc_modulo(a, b, &val);
            break;
        case '^':
            val = calc_power(a, b);
            break;
        default:
            return CALC_ERR_INVALID_OP;
    }

    if (status != CALC_OK) {
        return status;
    }
    if (!isfinite(val)) {
        return CALC_ERR_MATH_RANGE;
    }

    *result = val;
    return CALC_OK;
}
