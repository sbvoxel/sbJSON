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

static void parse_with_opts_should_handle_null(void) {
    const char *error_pointer = NULL;
    sbJSON *item = NULL;
    TEST_ASSERT_NULL_MESSAGE(sbj_parse_with_opts(NULL, &error_pointer, false),
                             "Failed to handle NULL input.");
    item = sbj_parse_with_opts("{}", NULL, false);
    TEST_ASSERT_NOT_NULL_MESSAGE(item, "Failed to handle NULL error pointer.");
    sbj_delete(item);
    TEST_ASSERT_NULL_MESSAGE(sbj_parse_with_opts(NULL, NULL, false),
                             "Failed to handle both NULL.");
    TEST_ASSERT_NULL_MESSAGE(
        sbj_parse_with_opts("{", NULL, false),
        "Failed to handle NULL error pointer with parse error.");
}

static void parse_with_opts_should_handle_empty_strings(void) {
    const char empty_string[] = "";
    const char *error_pointer = NULL;

    TEST_ASSERT_NULL(sbj_parse_with_opts(empty_string, NULL, false));
    TEST_ASSERT_EQUAL_PTR(empty_string, sbJSON_GetErrorPtr());

    TEST_ASSERT_NULL(sbj_parse_with_opts(empty_string, &error_pointer, false));
    TEST_ASSERT_EQUAL_PTR(empty_string, error_pointer);
    TEST_ASSERT_EQUAL_PTR(empty_string, sbJSON_GetErrorPtr());
}

static void parse_with_opts_should_handle_incomplete_json(void) {
    const char json[] = "{ \"name\": ";
    const char *parse_end = NULL;

    TEST_ASSERT_NULL(sbj_parse_with_opts(json, &parse_end, false));
    TEST_ASSERT_EQUAL_PTR(json + strlen(json), parse_end);
    TEST_ASSERT_EQUAL_PTR(json + strlen(json), sbJSON_GetErrorPtr());
}

static void parse_with_opts_should_require_null_if_requested(void) {
    sbJSON *item = sbj_parse_with_opts("{}", NULL, true);
    TEST_ASSERT_NOT_NULL(item);
    sbj_delete(item);
    item = sbj_parse_with_opts("{} \n", NULL, true);
    TEST_ASSERT_NOT_NULL(item);
    sbj_delete(item);
    TEST_ASSERT_NULL(sbj_parse_with_opts("{}x", NULL, true));
}

static void parse_with_opts_should_return_parse_end(void) {
    const char json[] = "[] empty array XD";
    const char *parse_end = NULL;

    sbJSON *item = sbj_parse_with_opts(json, &parse_end, false);
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_EQUAL_PTR(json + 2, parse_end);
    sbj_delete(item);
}

static void parse_with_opts_should_parse_utf8_bom(void) {
    sbJSON *with_bom = NULL;
    sbJSON *without_bom = NULL;

    with_bom = sbj_parse_with_opts("\xEF\xBB\xBF{}", NULL, true);
    TEST_ASSERT_NOT_NULL(with_bom);
    without_bom = sbj_parse_with_opts("{}", NULL, true);
    TEST_ASSERT_NOT_NULL(with_bom);

    TEST_ASSERT_TRUE(sbj_compare(with_bom, without_bom));

    sbj_delete(with_bom);
    sbj_delete(without_bom);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(parse_with_opts_should_handle_null);
    RUN_TEST(parse_with_opts_should_handle_empty_strings);
    RUN_TEST(parse_with_opts_should_handle_incomplete_json);
    RUN_TEST(parse_with_opts_should_require_null_if_requested);
    RUN_TEST(parse_with_opts_should_return_parse_end);
    RUN_TEST(parse_with_opts_should_parse_utf8_bom);

    return UNITY_END();
}
