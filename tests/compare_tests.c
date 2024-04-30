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

static bool compare_from_string(const char *const a, const char *const b) {
    sbJSON *a_json = NULL;
    sbJSON *b_json = NULL;
    bool result = false;

    a_json = sbj_parse(a);
    TEST_ASSERT_NOT_NULL_MESSAGE(a_json, "Failed to parse a.");
    b_json = sbj_parse(b);
    TEST_ASSERT_NOT_NULL_MESSAGE(b_json, "Failed to parse b.");

    result = sbj_compare(a_json, b_json);

    sbj_delete(a_json);
    sbj_delete(b_json);

    return result;
}

static void sbjson_compare_should_compare_null_pointer_as_equal(void) {
    TEST_ASSERT_TRUE(sbj_compare(NULL, NULL));
}

static void sbjson_compare_should_compare_invalid_as_equal(void) {
    sbJSON invalid1;
    sbJSON invalid2;
    memset(&invalid1, '\0', sizeof(invalid1));
    memset(&invalid2, '\0', sizeof(invalid2));

    TEST_ASSERT_TRUE(sbj_compare(&invalid1, &invalid1));
    TEST_ASSERT_TRUE(sbj_compare(&invalid1, &invalid2));
}

static void sbjson_compare_should_compare_numbers(void) {
    TEST_ASSERT_TRUE(compare_from_string("1", "1"));
    TEST_ASSERT_TRUE(compare_from_string("0.0001", "0.0001"));
    TEST_ASSERT_TRUE(compare_from_string("1E100", "10E99"));

    TEST_ASSERT_FALSE(compare_from_string("0.5E-100", "0.5E-101"));

    TEST_ASSERT_FALSE(compare_from_string("1", "2"));
}

static void sbjson_compare_should_compare_booleans(void) {
    /* true */
    TEST_ASSERT_TRUE(compare_from_string("true", "true"));

    /* false */
    TEST_ASSERT_TRUE(compare_from_string("false", "false"));

    /* mixed */
    TEST_ASSERT_FALSE(compare_from_string("true", "false"));
    TEST_ASSERT_FALSE(compare_from_string("false", "true"));
}

static void sbjson_compare_should_compare_null(void) {
    TEST_ASSERT_TRUE(compare_from_string("null", "null"));
    TEST_ASSERT_FALSE(compare_from_string("null", "true"));
}

static void sbjson_compare_should_compare_strings(void) {
    TEST_ASSERT_TRUE(compare_from_string("\"abcdefg\"", "\"abcdefg\""));
    TEST_ASSERT_FALSE(compare_from_string("\"ABCDEFG\"", "\"abcdefg\""));
}

static void sbjson_compare_should_compare_raw(void) {
    sbJSON *raw1 = NULL;
    sbJSON *raw2 = NULL;

    raw1 = sbj_parse("\"[true, false]\"");
    TEST_ASSERT_NOT_NULL(raw1);
    raw2 = sbj_parse("\"[true, false]\"");
    TEST_ASSERT_NOT_NULL(raw2);

    raw1->type = sbJSON_Raw;
    raw2->type = sbJSON_Raw;

    TEST_ASSERT_TRUE(sbj_compare(raw1, raw2));

    sbj_delete(raw1);
    sbj_delete(raw2);
}

static void sbjson_compare_should_compare_arrays(void) {
    TEST_ASSERT_TRUE(compare_from_string("[]", "[]"));

    TEST_ASSERT_TRUE(compare_from_string(
        "[false,true,null,42,\"string\",[],{}]",
        "[false, true, null, 42, \"string\", [], {}]"));

    TEST_ASSERT_TRUE(compare_from_string("[[[1], 2]]", "[[[1], 2]]"));

    TEST_ASSERT_FALSE(compare_from_string(
        "[true,null,42,\"string\",[],{}]",
        "[false, true, null, 42, \"string\", [], {}]"));

    /* Arrays that are a prefix of another array */
    TEST_ASSERT_FALSE(compare_from_string("[1,2,3]", "[1,2]"));
}

static void sbjson_compare_should_compare_objects(void) {
    TEST_ASSERT_TRUE(compare_from_string("{}", "{}"));

    //TODO:
    TEST_ASSERT_TRUE(compare_from_string(
        "{\"false\": false, \"true\": true, \"null\": null, \"number\": 42, "
        "\"string\": \"string\", \"array\": [], \"object\": {}}",
        "{\"true\": true, \"false\": false, \"null\": null, \"number\": 42, "
        "\"string\": \"string\", \"array\": [], \"object\": {}}"));
    TEST_ASSERT_FALSE(compare_from_string(
        "{\"False\": false, \"true\": true, \"null\": null, \"number\": 42, "
        "\"string\": \"string\", \"array\": [], \"object\": {}}",
        "{\"true\": true, \"false\": false, \"null\": null, \"number\": 42, "
        "\"string\": \"string\", \"array\": [], \"object\": {}}"));
    /* test objects that are a subset of each other */
    TEST_ASSERT_FALSE(
        compare_from_string("{\"one\": 1, \"two\": 2}",
                            "{\"one\": 1, \"two\": 2, \"three\": 3}"));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(sbjson_compare_should_compare_null_pointer_as_equal);
    RUN_TEST(sbjson_compare_should_compare_invalid_as_equal);
    RUN_TEST(sbjson_compare_should_compare_numbers);
    RUN_TEST(sbjson_compare_should_compare_booleans);
    RUN_TEST(sbjson_compare_should_compare_null);
    RUN_TEST(sbjson_compare_should_compare_strings);
    RUN_TEST(sbjson_compare_should_compare_raw);
    RUN_TEST(sbjson_compare_should_compare_arrays);
    RUN_TEST(sbjson_compare_should_compare_objects);

    return UNITY_END();
}
