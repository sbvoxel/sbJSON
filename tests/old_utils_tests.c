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

#include "../sbJSON_Utils.h"
#include "common.h"
#include "unity.h"

/* JSON Apply Merge tests: */
static const char *merges[15][3] = {
    {"{\"a\":\"b\"}", "{\"a\":\"c\"}", "{\"a\":\"c\"}"},
    {"{\"a\":\"b\"}", "{\"b\":\"c\"}", "{\"a\":\"b\",\"b\":\"c\"}"},
    {"{\"a\":\"b\"}", "{\"a\":null}", "{}"},
    {"{\"a\":\"b\",\"b\":\"c\"}", "{\"a\":null}", "{\"b\":\"c\"}"},
    {"{\"a\":[\"b\"]}", "{\"a\":\"c\"}", "{\"a\":\"c\"}"},
    {"{\"a\":\"c\"}", "{\"a\":[\"b\"]}", "{\"a\":[\"b\"]}"},
    {"{\"a\":{\"b\":\"c\"}}", "{\"a\":{\"b\":\"d\",\"c\":null}}",
     "{\"a\":{\"b\":\"d\"}}"},
    {"{\"a\":[{\"b\":\"c\"}]}", "{\"a\":[1]}", "{\"a\":[1]}"},
    {"[\"a\",\"b\"]", "[\"c\",\"d\"]", "[\"c\",\"d\"]"},
    {"{\"a\":\"b\"}", "[\"c\"]", "[\"c\"]"},
    {"{\"a\":\"foo\"}", "null", "null"},
    {"{\"a\":\"foo\"}", "\"bar\"", "\"bar\""},
    {"{\"e\":null}", "{\"a\":1}", "{\"e\":null,\"a\":1}"},
    {"[1,2]", "{\"a\":\"b\",\"c\":null}", "{\"a\":\"b\"}"},
    {"{}", "{\"a\":{\"bb\":{\"ccc\":null}}}", "{\"a\":{\"bb\":{}}}"}};

static void json_pointer_tests(void) {
    sbJSON *root = NULL;
    const char *json = "{"
                       "\"foo\": [\"bar\", \"baz\"],"
                       "\"\": 0,"
                       "\"a/b\": 1,"
                       "\"c%d\": 2,"
                       "\"e^f\": 3,"
                       "\"g|h\": 4,"
                       "\"i\\\\j\": 5,"
                       "\"k\\\"l\": 6,"
                       "\" \": 7,"
                       "\"m~n\": 8"
                       "}";

    root = sbJSON_Parse(json);

    TEST_ASSERT_EQUAL_PTR(sbJSONUtils_GetPointer(root, ""), root);
    TEST_ASSERT_EQUAL_PTR(sbJSONUtils_GetPointer(root, "/foo"),
                          sbJSON_GetObjectItem(root, "foo"));
    TEST_ASSERT_EQUAL_PTR(sbJSONUtils_GetPointer(root, "/foo/0"),
                          sbJSON_GetObjectItem(root, "foo")->child);
    TEST_ASSERT_EQUAL_PTR(sbJSONUtils_GetPointer(root, "/foo/0"),
                          sbJSON_GetObjectItem(root, "foo")->child);
    TEST_ASSERT_EQUAL_PTR(sbJSONUtils_GetPointer(root, "/"),
                          sbJSON_GetObjectItem(root, ""));
    TEST_ASSERT_EQUAL_PTR(sbJSONUtils_GetPointer(root, "/a~1b"),
                          sbJSON_GetObjectItem(root, "a/b"));
    TEST_ASSERT_EQUAL_PTR(sbJSONUtils_GetPointer(root, "/c%d"),
                          sbJSON_GetObjectItem(root, "c%d"));
    TEST_ASSERT_EQUAL_PTR(sbJSONUtils_GetPointer(root, "/c^f"),
                          sbJSON_GetObjectItem(root, "c^f"));
    TEST_ASSERT_EQUAL_PTR(sbJSONUtils_GetPointer(root, "/c|f"),
                          sbJSON_GetObjectItem(root, "c|f"));
    TEST_ASSERT_EQUAL_PTR(sbJSONUtils_GetPointer(root, "/i\\j"),
                          sbJSON_GetObjectItem(root, "i\\j"));
    TEST_ASSERT_EQUAL_PTR(sbJSONUtils_GetPointer(root, "/k\"l"),
                          sbJSON_GetObjectItem(root, "k\"l"));
    TEST_ASSERT_EQUAL_PTR(sbJSONUtils_GetPointer(root, "/ "),
                          sbJSON_GetObjectItem(root, " "));
    TEST_ASSERT_EQUAL_PTR(sbJSONUtils_GetPointer(root, "/m~0n"),
                          sbJSON_GetObjectItem(root, "m~n"));

    sbJSON_Delete(root);
}

static void misc_tests(void) {
    /* Misc tests */
    int numbers[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    sbJSON *object = NULL;
    sbJSON *object1 = NULL;
    sbJSON *object2 = NULL;
    sbJSON *object3 = NULL;
    sbJSON *object4 = NULL;
    sbJSON *nums = NULL;
    sbJSON *num6 = NULL;
    char *pointer = NULL;

    printf("JSON Pointer construct\n");
    object = sbJSON_CreateObject();
    nums = sbJSON_CreateIntArray(numbers, 10);
    num6 = sbJSON_GetArrayItem(nums, 6);
    sbJSON_AddItemToObject(object, "numbers", nums);

    pointer = sbJSONUtils_FindPointerFromObjectTo(object, num6);
    TEST_ASSERT_EQUAL_STRING("/numbers/6", pointer);
    free(pointer);

    pointer = sbJSONUtils_FindPointerFromObjectTo(object, nums);
    TEST_ASSERT_EQUAL_STRING("/numbers", pointer);
    free(pointer);

    pointer = sbJSONUtils_FindPointerFromObjectTo(object, object);
    TEST_ASSERT_EQUAL_STRING("", pointer);
    free(pointer);

    object1 = sbJSON_CreateObject();
    object2 = sbJSON_CreateString("m~n");
    sbJSON_AddItemToObject(object1, "m~n", object2);
    pointer = sbJSONUtils_FindPointerFromObjectTo(object1, object2);
    TEST_ASSERT_EQUAL_STRING("/m~0n", pointer);
    free(pointer);

    object3 = sbJSON_CreateObject();
    object4 = sbJSON_CreateString("m/n");
    sbJSON_AddItemToObject(object3, "m/n", object4);
    pointer = sbJSONUtils_FindPointerFromObjectTo(object3, object4);
    TEST_ASSERT_EQUAL_STRING("/m~1n", pointer);
    free(pointer);

    sbJSON_Delete(object);
    sbJSON_Delete(object1);
    sbJSON_Delete(object3);
}

static void sort_tests(void) {
    /* Misc tests */
    const char *random = "QWERTYUIOPASDFGHJKLZXCVBNM";
    char buf[2] = {'\0', '\0'};
    sbJSON *sortme = NULL;
    size_t i = 0;
    sbJSON *current_element = NULL;

    /* JSON Sort test: */
    sortme = sbJSON_CreateObject();
    for (i = 0; i < 26; i++) {
        buf[0] = random[i];
        sbJSON_AddItemToObject(sortme, buf, sbJSON_CreateNumber(1));
    }

    sbJSONUtils_SortObject(sortme);

    /* check sorting */
    current_element = sortme->child->next;
    for (i = 1; (i < 26) && (current_element != NULL) &&
                (current_element->prev != NULL);
         i++) {
        TEST_ASSERT_TRUE(current_element->string[0] >=
                         current_element->prev->string[0]);
        current_element = current_element->next;
    }

    sbJSON_Delete(sortme);
}

static void merge_tests(void) {
    size_t i = 0;
    char *patchtext = NULL;
    char *after = NULL;

    /* Merge tests: */
    printf("JSON Merge Patch tests\n");
    for (i = 0; i < 15; i++) {
        sbJSON *object_to_be_merged = sbJSON_Parse(merges[i][0]);
        sbJSON *patch = sbJSON_Parse(merges[i][1]);
        patchtext = sbJSON_PrintUnformatted(patch);
        object_to_be_merged = sbJSONUtils_MergePatch(object_to_be_merged, patch);
        after = sbJSON_PrintUnformatted(object_to_be_merged);
        TEST_ASSERT_EQUAL_STRING(merges[i][2], after);

        free(patchtext);
        free(after);
        sbJSON_Delete(object_to_be_merged);
        sbJSON_Delete(patch);
    }
}

static void generate_merge_tests(void) {
    size_t i = 0;
    char *patchedtext = NULL;

    /* Generate Merge tests: */
    for (i = 0; i < 15; i++) {
        sbJSON *from = sbJSON_Parse(merges[i][0]);
        sbJSON *to = sbJSON_Parse(merges[i][2]);
        sbJSON *patch = sbJSONUtils_GenerateMergePatch(from, to);
        from = sbJSONUtils_MergePatch(from, patch);
        patchedtext = sbJSON_PrintUnformatted(from);
        TEST_ASSERT_EQUAL_STRING(merges[i][2], patchedtext);

        sbJSON_Delete(from);
        sbJSON_Delete(to);
        sbJSON_Delete(patch);
        free(patchedtext);
    }
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(json_pointer_tests);
    RUN_TEST(misc_tests);
    RUN_TEST(sort_tests);
    RUN_TEST(merge_tests);
    RUN_TEST(generate_merge_tests);

    return UNITY_END();
}
