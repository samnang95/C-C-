#include "calculator/calculator.h"

const char *calc_status_message(CalcStatus status) {
    switch (status) {
        case CALC_OK:
            return "OK";
        case CALC_ERR_DIV_BY_ZERO:
            return "Error: division by zero is not allowed.";
        case CALC_ERR_INVALID_OP:
            return "Error: unsupported operator (use +, -, x, /, %, ^).";
        case CALC_ERR_NULL_PTR:
            return "Error: internal null pointer passed for result.";
        case CALC_ERR_MATH_RANGE:
            return "Error: result is undefined or out of floating-point range.";
        default:
            return "Error: unknown calculation error.";
    }
}
