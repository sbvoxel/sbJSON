/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors
  Modifications (c) 2024 sbJSON Authors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "common.h"
#include "unity.h"

static void assert_print_number(const char *expected, bool is_double, double double_input, int integer_input) {
    unsigned char printed[1024];
    unsigned char new_buffer[26];
    unsigned int i = 0;
    sbJSON item[1];
    printbuffer buffer = {0, 0, 0, 0, 0, 0, {0, 0, 0}};
    buffer.buffer = printed;
    buffer.length = sizeof(printed);
    buffer.offset = 0;
    buffer.noalloc = true;
    buffer.hooks = global_hooks;
    buffer.buffer = new_buffer;

    memset(item, 0, sizeof(item));
    memset(new_buffer, 0, sizeof(new_buffer));
    if (is_double) {
        sbJSON_SetDoubleNumberValue(item, double_input);
    } else {
        sbJSON_SetIntegerNumberValue(item, integer_input);
    }
    TEST_ASSERT_TRUE_MESSAGE(print_number(item, &buffer),
                             "Failed to print number.");

    /* In MinGW or visual studio(before 2015),the exponten is represented using
     * three digits,like:"1e-009","1e+017" remove extra "0" to output "1e-09" or
     * "1e+17",which makes testcase PASS */
    for (i = 0; i < sizeof(new_buffer); i++) {
        if (i > 3 && new_buffer[i] == '0') {
            if ((new_buffer[i - 3] == 'e' && new_buffer[i - 2] == '-' &&
                 new_buffer[i] == '0') ||
                (new_buffer[i - 2] == 'e' && new_buffer[i - 1] == '+')) {
                while (new_buffer[i] != '\0') {
                    new_buffer[i] = new_buffer[i + 1];
                    i++;
                }
            }
        }
    }
    TEST_ASSERT_EQUAL_STRING_MESSAGE(expected, buffer.buffer,
                                     "Printed number is not as expected.");
}

static void assert_print_integer_number(const char *expected, int input) {
    assert_print_number(expected, false, 0, input);
}

static void assert_print_double_number(const char *expected, double input) {
    assert_print_number(expected, true, input, 0);
}

static void print_number_should_print_zero(void) {
    assert_print_integer_number("0", 0);
}

static void print_number_should_print_negative_integers(void) {
    assert_print_integer_number("-1", -1.0);
    assert_print_integer_number("-32768", -32768.0);
    assert_print_integer_number("-2147483648", -2147483648.0);
}

static void print_number_should_print_positive_integers(void) {
    assert_print_integer_number("1", 1);
    assert_print_integer_number("32767", 32767);
    assert_print_integer_number("2147483647", 2147483647);
}

static void print_number_should_print_positive_reals(void) {
    assert_print_double_number("0.123", 0.123);
    assert_print_double_number("1e-09", 10e-10);
    assert_print_double_number("1000000000000", 10e11);
    assert_print_double_number("1.23e+129", 123e+127);
    assert_print_double_number("1.23e-126", 123e-128);
    assert_print_double_number("3.1415926535897931", 3.1415926535897931);
}

static void print_number_should_print_negative_reals(void) {
    assert_print_double_number("-0.0123", -0.0123);
    assert_print_double_number("-1e-09", -10e-10);
    assert_print_double_number("-1e+21", -10e20);
    assert_print_double_number("-1.23e+129", -123e+127);
    assert_print_double_number("-1.23e-126", -123e-128);
}

static void print_number_should_print_non_number(void) {
    TEST_IGNORE();
     assert_print_double_number("null", NAN);
     assert_print_double_number("null", INFINITY);
     assert_print_double_number("null", -INFINITY);
}

int main(void) {
    /* initialize sbJSON item */
    UNITY_BEGIN();

    RUN_TEST(print_number_should_print_zero);
    RUN_TEST(print_number_should_print_negative_integers);
    RUN_TEST(print_number_should_print_positive_integers);
    RUN_TEST(print_number_should_print_positive_reals);
    RUN_TEST(print_number_should_print_negative_reals);
    RUN_TEST(print_number_should_print_non_number);

    return UNITY_END();
}
