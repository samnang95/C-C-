#include "calculator/calculator.h"

#include <math.h>
#include <stddef.h>

CalcStatus calc_divide(double a, double b, double *result) {
    if (result == NULL) {
        return CALC_ERR_NULL_PTR;
    }
    if (b == 0.0) {
        return CALC_ERR_DIV_BY_ZERO;
    }

    double val = a / b;
    if (!isfinite(val)) {
        return CALC_ERR_MATH_RANGE;
    }

    *result = val;
    return CALC_OK;
}

CalcStatus calc_modulo(double a, double b, double *result) {
    if (result == NULL) {
        return CALC_ERR_NULL_PTR;
    }
    if (b == 0.0) {
        return CALC_ERR_DIV_BY_ZERO;
    }

    double val = fmod(a, b);
    if (!isfinite(val)) {
        return CALC_ERR_MATH_RANGE;
    }

    *result = val;
    return CALC_OK;
}

double calc_power(double a, double b) {
    return pow(a, b);
}
