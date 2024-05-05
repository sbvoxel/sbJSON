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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../sbjson_utils.h"
#include "common.h"
#include "unity.h"

static sbJSON *parse_test_file(const char *const filename) {
    char *file = NULL;
    sbJSON *json = NULL;

    file = read_file(filename);
    TEST_ASSERT_NOT_NULL_MESSAGE(file, "Failed to read file.");

    json = sbj_parse(file);
    TEST_ASSERT_NOT_NULL_MESSAGE(json, "Failed to parse test json.");
    TEST_ASSERT_TRUE_MESSAGE(sbj_is_array(json), "Json is not an array.");

    free(file);

    return json;
}

static bool test_apply_patch(const sbJSON *const test) {
    sbJSON *doc = NULL;
    sbJSON *patch = NULL;
    sbJSON *expected = NULL;
    sbJSON *error_element = NULL;
    sbJSON *comment = NULL;
    sbJSON *disabled = NULL;

    sbJSON *object = NULL;
    bool successful = false;

    /* extract all the data out of the test */
    comment = sbj_get_object_item(test, "comment");
    if (sbj_is_string(comment)) {
        printf("Testing \"%s\"\n", comment->u.valuestring);
    } else {
        printf("Testing unknown\n");
    }

    disabled = sbj_get_object_item(test, "disabled");
    if (sbj_is_bool(disabled) && sbj_get_bool_value(disabled)) {
        printf("SKIPPED\n");
        return true;
    }

    doc = sbj_get_object_item(test, "doc");
    TEST_ASSERT_NOT_NULL_MESSAGE(doc, "No \"doc\" in the test.");
    patch = sbj_get_object_item(test, "patch");
    TEST_ASSERT_NOT_NULL_MESSAGE(patch, "No \"patch\"in the test.");
    /* Make a working copy of 'doc' */
    object = sbj_duplicate(doc, true);
    TEST_ASSERT_NOT_NULL(object);

    expected = sbj_get_object_item(test, "expected");
    error_element = sbj_get_object_item(test, "error");
    if (error_element != NULL) {
        /* excepting an error */
        TEST_ASSERT_TRUE_MESSAGE(
            0 != sbJSONUtils_ApplyPatches(object, patch),
            "Test didn't fail as it's supposed to.");

        successful = true;
    } else {
        /* apply the patch */
        TEST_ASSERT_EQUAL_INT_MESSAGE(
            0, sbJSONUtils_ApplyPatches(object, patch),
            "Failed to apply patches.");
        successful = true;

        if (expected != NULL) {
            successful = sbj_compare(object, expected);
        }
    }

    sbj_delete(object);

    if (successful) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
    }

    return successful;
}

static bool test_generate_test(sbJSON *test) {
    sbJSON *doc = NULL;
    sbJSON *patch = NULL;
    sbJSON *expected = NULL;
    sbJSON *disabled = NULL;

    sbJSON *object = NULL;
    bool successful = false;

    char *printed_patch = NULL;

    disabled = sbj_get_object_item(test, "disabled");
    if (sbj_is_bool(disabled) && sbj_get_bool_value(disabled)) {
        printf("SKIPPED\n");
        return true;
    }

    doc = sbj_get_object_item(test, "doc");
    TEST_ASSERT_NOT_NULL_MESSAGE(doc, "No \"doc\" in the test.");

    /* Make a working copy of 'doc' */
    object = sbj_duplicate(doc, true);
    TEST_ASSERT_NOT_NULL(object);

    expected = sbj_get_object_item(test, "expected");
    if (expected == NULL) {
        sbj_delete(object);
        /* if there is no expected output, this test doesn't make sense */
        return true;
    }

    patch = sbJSONUtils_GeneratePatches(doc, expected);
    TEST_ASSERT_NOT_NULL_MESSAGE(patch, "Failed to generate patches.");

    printed_patch = sbj_print(patch);
    printf("%s\n", printed_patch);
    free(printed_patch);

    /* apply the generated patch */
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        0, sbJSONUtils_ApplyPatches(object, patch),
        "Failed to apply generated patch.");

    successful = sbj_compare(object, expected);

    sbj_delete(patch);
    sbj_delete(object);

    if (successful) {
        printf("generated patch: OK\n");
    } else {
        printf("generated patch: FAILED\n");
    }

    return successful;
}

static void sbjson_utils_should_pass_json_patch_test_tests(void) {
    sbJSON *tests = parse_test_file("json-patch-tests/tests.json");
    sbJSON *test = NULL;

    bool failed = false;
    sbJSON_ArrayForEach(test, tests) {
        failed |= !test_apply_patch(test);
        failed |= !test_generate_test(test);
    }

    sbj_delete(tests);

    TEST_ASSERT_FALSE_MESSAGE(failed, "Some tests failed.");
}

static void sbjson_utils_should_pass_json_patch_test_spec_tests(void) {
    sbJSON *tests = parse_test_file("json-patch-tests/spec_tests.json");
    sbJSON *test = NULL;

    bool failed = false;
    sbJSON_ArrayForEach(test, tests) {
        failed |= !test_apply_patch(test);
        failed |= !test_generate_test(test);
    }

    sbj_delete(tests);

    TEST_ASSERT_FALSE_MESSAGE(failed, "Some tests failed.");
}

static void sbjson_utils_should_pass_json_patch_test_sbjson_utils_tests(void) {
    sbJSON *tests = parse_test_file("json-patch-tests/sbjson-utils-tests.json");
    sbJSON *test = NULL;

    bool failed = false;
    sbJSON_ArrayForEach(test, tests) {
        failed |= !test_apply_patch(test);
        failed |= !test_generate_test(test);
    }

    sbj_delete(tests);

    TEST_ASSERT_FALSE_MESSAGE(failed, "Some tests failed.");
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(sbjson_utils_should_pass_json_patch_test_tests);
    RUN_TEST(sbjson_utils_should_pass_json_patch_test_spec_tests);
    RUN_TEST(sbjson_utils_should_pass_json_patch_test_sbjson_utils_tests);

    return UNITY_END();
}
