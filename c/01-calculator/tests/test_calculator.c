#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "calculator/calculator.h"
#include "input/input.h"

#define EPSILON 1e-9

static int approx_equal(double a, double b) {
    return fabs(a - b) < EPSILON;
}

static void test_basic_operations(void) {
    double result = 0.0;

    assert(calc_evaluate(5.0, '+', 3.0, &result) == CALC_OK);
    assert(approx_equal(result, 8.0));

    assert(calc_evaluate(10.5, '-', 4.5, &result) == CALC_OK);
    assert(approx_equal(result, 6.0));

    assert(calc_evaluate(-4.0, '*', 2.5, &result) == CALC_OK);
    assert(approx_equal(result, -10.0));

    assert(calc_evaluate(6.0, 'x', 7.0, &result) == CALC_OK);
    assert(approx_equal(result, 42.0));

    assert(calc_evaluate(6.0, 'X', 7.0, &result) == CALC_OK);
    assert(approx_equal(result, 42.0));
}

static void test_advanced_operations(void) {
    double result = 0.0;

    assert(calc_evaluate(7.0, '/', 2.0, &result) == CALC_OK);
    assert(approx_equal(result, 3.5));

    assert(calc_evaluate(9.0, '\\', 2.0, &result) == CALC_OK);
    assert(approx_equal(result, 4.5));

    assert(calc_evaluate(10.0, '%', 3.0, &result) == CALC_OK);
    assert(approx_equal(result, 1.0));

    assert(calc_evaluate(2.0, '^', 8.0, &result) == CALC_OK);
    assert(approx_equal(result, 256.0));
}

static void test_error_and_null_handling(void) {
    double result = 123.0;

    /* Division & modulo by zero must not clobber result */
    assert(calc_evaluate(10.0, '/', 0.0, &result) == CALC_ERR_DIV_BY_ZERO);
    assert(approx_equal(result, 123.0));

    assert(calc_evaluate(10.0, '%', 0.0, &result) == CALC_ERR_DIV_BY_ZERO);
    assert(approx_equal(result, 123.0));

    /* Invalid operator */
    assert(calc_evaluate(10.0, '?', 2.0, &result) == CALC_ERR_INVALID_OP);
    assert(!calc_is_valid_op('?'));
    assert(calc_is_valid_op('+'));
    assert(calc_is_valid_op('\\'));

    /* NULL pointer safety */
    assert(calc_evaluate(10.0, '+', 2.0, NULL) == CALC_ERR_NULL_PTR);
    assert(calc_divide(10.0, 2.0, NULL) == CALC_ERR_NULL_PTR);
    assert(calc_modulo(10.0, 3.0, NULL) == CALC_ERR_NULL_PTR);

    /* Non-finite / overflow / domain errors */
    assert(calc_evaluate(1e308, '*', 10.0, &result) == CALC_ERR_MATH_RANGE);
    assert(calc_evaluate(2.0, '^', 2000.0, &result) == CALC_ERR_MATH_RANGE);
    assert(calc_evaluate(-4.0, '^', 0.5, &result) == CALC_ERR_MATH_RANGE);
    assert(calc_evaluate(NAN, '+', 1.0, &result) == CALC_ERR_MATH_RANGE);

    /* Status messages are non-NULL and non-empty */
    assert(strlen(calc_status_message(CALC_OK)) > 0);
    assert(strlen(calc_status_message(CALC_ERR_NULL_PTR)) > 0);
    assert(strlen(calc_status_message(CALC_ERR_MATH_RANGE)) > 0);
}

static void test_number_parsing(void) {
    double val = 0.0;

    assert(parse_number("42.5", &val) == 1);
    assert(approx_equal(val, 42.5));

    assert(parse_number("  -1.25e2  ", &val) == 1);
    assert(approx_equal(val, -125.0));

    /* Reject malformed, empty, NULL, overflow, underflow, NaN, and Inf */
    assert(parse_number(NULL, &val) == 0);
    assert(parse_number("10", NULL) == 0);
    assert(parse_number("", &val) == 0);
    assert(parse_number("   ", &val) == 0);
    assert(parse_number("12abc", &val) == 0);
    assert(parse_number("1e500", &val) == 0);
    assert(parse_number("1e-500", &val) == 0);
    assert(parse_number("nan", &val) == 0);
    assert(parse_number("inf", &val) == 0);
    assert(parse_number("-infinity", &val) == 0);

    /* Direct NULL guard checks on interactive readers */
    assert(read_number("Prompt: ", NULL) == INPUT_ERR_NULL);
    assert(read_operator(NULL) == INPUT_ERR_NULL);
}

int main(void) {
    test_basic_operations();
    test_advanced_operations();
    test_error_and_null_handling();
    test_number_parsing();

    printf("All calculator and input unit tests passed!\n");
    return 0;
}

