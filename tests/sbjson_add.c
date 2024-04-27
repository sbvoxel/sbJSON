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

#include "common.h"
#include "unity.h"

static void *failing_malloc(size_t size) {
    (void)size;
    return NULL;
}

/* work around MSVC error C2322: '...' address of dillimport '...' is not static
 */
static void normal_free(void *pointer) { free(pointer); }

static sbJSON_Hooks failing_hooks = {failing_malloc, normal_free};

static void sbjson_add_null_should_add_null(void) {
    sbJSON *root = sbJSON_CreateObject();
    sbJSON *null = NULL;

    sbJSON_AddNullToObject(root, "null");

    TEST_ASSERT_NOT_NULL(null = sbJSON_GetObjectItemCaseSensitive(root, "null"));
    TEST_ASSERT_EQUAL_INT(null->type, sbJSON_Null);

    sbJSON_Delete(root);
}

static void sbjson_add_null_should_fail_with_null_pointers(void) {
    sbJSON *root = sbJSON_CreateObject();

    TEST_ASSERT_NULL(sbJSON_AddNullToObject(NULL, "null"));
    TEST_ASSERT_NULL(sbJSON_AddNullToObject(root, NULL));

    sbJSON_Delete(root);
}

static void sbjson_add_null_should_fail_on_allocation_failure(void) {
    sbJSON *root = sbJSON_CreateObject();

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbJSON_AddNullToObject(root, "null"));

    sbJSON_InitHooks(NULL);

    sbJSON_Delete(root);
}

static void sbjson_add_true_should_add_true(void) {
    sbJSON *root = sbJSON_CreateObject();
    sbJSON *true_item = NULL;

    sbJSON_AddTrueToObject(root, "true");

    TEST_ASSERT_NOT_NULL(true_item =
                             sbJSON_GetObjectItemCaseSensitive(root, "true"));
    TEST_ASSERT_EQUAL_INT(true_item->type, sbJSON_True);

    sbJSON_Delete(root);
}

static void sbjson_add_true_should_fail_with_null_pointers(void) {
    sbJSON *root = sbJSON_CreateObject();

    TEST_ASSERT_NULL(sbJSON_AddTrueToObject(NULL, "true"));
    TEST_ASSERT_NULL(sbJSON_AddTrueToObject(root, NULL));

    sbJSON_Delete(root);
}

static void sbjson_add_true_should_fail_on_allocation_failure(void) {
    sbJSON *root = sbJSON_CreateObject();

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbJSON_AddTrueToObject(root, "true"));

    sbJSON_InitHooks(NULL);

    sbJSON_Delete(root);
}

static void sbjson_create_int_array_should_fail_on_allocation_failure(void) {
    int numbers[] = {1, 2, 3};

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbJSON_CreateIntArray(numbers, 3));

    sbJSON_InitHooks(NULL);
}

static void sbjson_create_float_array_should_fail_on_allocation_failure(void) {
    float numbers[] = {1.0f, 2.0f, 3.0f};

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbJSON_CreateFloatArray(numbers, 3));

    sbJSON_InitHooks(NULL);
}

static void sbjson_create_double_array_should_fail_on_allocation_failure(void) {
    double numbers[] = {1.0, 2.0, 3.0};

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbJSON_CreateDoubleArray(numbers, 3));

    sbJSON_InitHooks(NULL);
}

static void sbjson_create_string_array_should_fail_on_allocation_failure(void) {
    const char *strings[] = {"1", "2", "3"};

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbJSON_CreateStringArray(strings, 3));

    sbJSON_InitHooks(NULL);
}

static void sbjson_add_false_should_add_false(void) {
    sbJSON *root = sbJSON_CreateObject();
    sbJSON *false_item = NULL;

    sbJSON_AddFalseToObject(root, "false");

    TEST_ASSERT_NOT_NULL(false_item =
                             sbJSON_GetObjectItemCaseSensitive(root, "false"));
    TEST_ASSERT_EQUAL_INT(false_item->type, sbJSON_False);

    sbJSON_Delete(root);
}

static void sbjson_add_false_should_fail_with_null_pointers(void) {
    sbJSON *root = sbJSON_CreateObject();

    TEST_ASSERT_NULL(sbJSON_AddFalseToObject(NULL, "false"));
    TEST_ASSERT_NULL(sbJSON_AddFalseToObject(root, NULL));

    sbJSON_Delete(root);
}

static void sbjson_add_false_should_fail_on_allocation_failure(void) {
    sbJSON *root = sbJSON_CreateObject();

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbJSON_AddFalseToObject(root, "false"));

    sbJSON_InitHooks(NULL);

    sbJSON_Delete(root);
}

static void sbjson_add_bool_should_add_bool(void) {
    sbJSON *root = sbJSON_CreateObject();
    sbJSON *true_item = NULL;
    sbJSON *false_item = NULL;

    /* true */
    sbJSON_AddBoolToObject(root, "true", true);
    TEST_ASSERT_NOT_NULL(true_item =
                             sbJSON_GetObjectItemCaseSensitive(root, "true"));
    TEST_ASSERT_EQUAL_INT(true_item->type, sbJSON_True);

    /* false */
    sbJSON_AddBoolToObject(root, "false", false);
    TEST_ASSERT_NOT_NULL(false_item =
                             sbJSON_GetObjectItemCaseSensitive(root, "false"));
    TEST_ASSERT_EQUAL_INT(false_item->type, sbJSON_False);

    sbJSON_Delete(root);
}

static void sbjson_add_bool_should_fail_with_null_pointers(void) {
    sbJSON *root = sbJSON_CreateObject();

    TEST_ASSERT_NULL(sbJSON_AddBoolToObject(NULL, "false", false));
    TEST_ASSERT_NULL(sbJSON_AddBoolToObject(root, NULL, false));

    sbJSON_Delete(root);
}

static void sbjson_add_bool_should_fail_on_allocation_failure(void) {
    sbJSON *root = sbJSON_CreateObject();

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbJSON_AddBoolToObject(root, "false", false));

    sbJSON_InitHooks(NULL);

    sbJSON_Delete(root);
}

static void sbjson_add_number_should_add_number(void) {
    sbJSON *root = sbJSON_CreateObject();
    sbJSON *number = NULL;

    sbJSON_AddIntegerNumberToObject(root, "number", 42);

    TEST_ASSERT_NOT_NULL(number =
                             sbJSON_GetObjectItemCaseSensitive(root, "number"));

    TEST_ASSERT_EQUAL_INT(number->type, sbJSON_Number);
    TEST_ASSERT_FALSE(number->is_number_double);
    TEST_ASSERT_EQUAL_INT(number->u.valueint, 42);

    sbJSON_Delete(root);
}

static void sbjson_add_number_should_fail_with_null_pointers(void) {
    sbJSON *root = sbJSON_CreateObject();

    TEST_ASSERT_NULL(sbJSON_AddIntegerNumberToObject(NULL, "number", 42));
    TEST_ASSERT_NULL(sbJSON_AddIntegerNumberToObject(root, NULL, 42));

    sbJSON_Delete(root);
}

static void sbjson_add_number_should_fail_on_allocation_failure(void) {
    sbJSON *root = sbJSON_CreateObject();

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbJSON_AddIntegerNumberToObject(root, "number", 42));

    sbJSON_InitHooks(NULL);

    sbJSON_Delete(root);
}

static void sbjson_add_string_should_add_string(void) {
    sbJSON *root = sbJSON_CreateObject();
    sbJSON *string = NULL;

    sbJSON_AddStringToObject(root, "string", "Hello World!");

    TEST_ASSERT_NOT_NULL(string =
                             sbJSON_GetObjectItemCaseSensitive(root, "string"));
    TEST_ASSERT_EQUAL_INT(string->type, sbJSON_String);
    TEST_ASSERT_EQUAL_STRING(string->u.valuestring, "Hello World!");

    sbJSON_Delete(root);
}

static void sbjson_add_string_should_fail_with_null_pointers(void) {
    sbJSON *root = sbJSON_CreateObject();

    TEST_ASSERT_NULL(sbJSON_AddStringToObject(NULL, "string", "string"));
    TEST_ASSERT_NULL(sbJSON_AddStringToObject(root, NULL, "string"));

    sbJSON_Delete(root);
}

static void sbjson_add_string_should_fail_on_allocation_failure(void) {
    sbJSON *root = sbJSON_CreateObject();

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbJSON_AddStringToObject(root, "string", "string"));

    sbJSON_InitHooks(NULL);

    sbJSON_Delete(root);
}

static void sbjson_add_raw_should_add_raw(void) {
    sbJSON *root = sbJSON_CreateObject();
    sbJSON *raw = NULL;

    sbJSON_AddRawToObject(root, "raw", "{}");

    TEST_ASSERT_NOT_NULL(raw = sbJSON_GetObjectItemCaseSensitive(root, "raw"));
    TEST_ASSERT_EQUAL_INT(raw->type, sbJSON_Raw);
    TEST_ASSERT_EQUAL_STRING(raw->u.valuestring, "{}");

    sbJSON_Delete(root);
}

static void sbjson_add_raw_should_fail_with_null_pointers(void) {
    sbJSON *root = sbJSON_CreateObject();

    TEST_ASSERT_NULL(sbJSON_AddRawToObject(NULL, "raw", "{}"));
    TEST_ASSERT_NULL(sbJSON_AddRawToObject(root, NULL, "{}"));

    sbJSON_Delete(root);
}

static void sbjson_add_raw_should_fail_on_allocation_failure(void) {
    sbJSON *root = sbJSON_CreateObject();

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbJSON_AddRawToObject(root, "raw", "{}"));

    sbJSON_InitHooks(NULL);

    sbJSON_Delete(root);
}

static void sbJSON_add_object_should_add_object(void) {
    sbJSON *root = sbJSON_CreateObject();
    sbJSON *object = NULL;

    sbJSON_AddObjectToObject(root, "object");
    TEST_ASSERT_NOT_NULL(object =
                             sbJSON_GetObjectItemCaseSensitive(root, "object"));
    TEST_ASSERT_EQUAL_INT(object->type, sbJSON_Object);

    sbJSON_Delete(root);
}

static void sbjson_add_object_should_fail_with_null_pointers(void) {
    sbJSON *root = sbJSON_CreateObject();

    TEST_ASSERT_NULL(sbJSON_AddObjectToObject(NULL, "object"));
    TEST_ASSERT_NULL(sbJSON_AddObjectToObject(root, NULL));

    sbJSON_Delete(root);
}

static void sbjson_add_object_should_fail_on_allocation_failure(void) {
    sbJSON *root = sbJSON_CreateObject();

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbJSON_AddObjectToObject(root, "object"));

    sbJSON_InitHooks(NULL);

    sbJSON_Delete(root);
}

static void sbJSON_add_array_should_add_array(void) {
    sbJSON *root = sbJSON_CreateObject();
    sbJSON *array = NULL;

    sbJSON_AddArrayToObject(root, "array");
    TEST_ASSERT_NOT_NULL(array =
                             sbJSON_GetObjectItemCaseSensitive(root, "array"));
    TEST_ASSERT_EQUAL_INT(array->type, sbJSON_Array);

    sbJSON_Delete(root);
}

static void sbjson_add_array_should_fail_with_null_pointers(void) {
    sbJSON *root = sbJSON_CreateObject();

    TEST_ASSERT_NULL(sbJSON_AddArrayToObject(NULL, "array"));
    TEST_ASSERT_NULL(sbJSON_AddArrayToObject(root, NULL));

    sbJSON_Delete(root);
}

static void sbjson_add_array_should_fail_on_allocation_failure(void) {
    sbJSON *root = sbJSON_CreateObject();

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbJSON_AddArrayToObject(root, "array"));

    sbJSON_InitHooks(NULL);

    sbJSON_Delete(root);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(sbjson_add_null_should_add_null);
    RUN_TEST(sbjson_add_null_should_fail_with_null_pointers);
    RUN_TEST(sbjson_add_null_should_fail_on_allocation_failure);

    RUN_TEST(sbjson_add_true_should_add_true);
    RUN_TEST(sbjson_add_true_should_fail_with_null_pointers);
    RUN_TEST(sbjson_add_true_should_fail_on_allocation_failure);

    RUN_TEST(sbjson_create_int_array_should_fail_on_allocation_failure);
    RUN_TEST(sbjson_create_float_array_should_fail_on_allocation_failure);
    RUN_TEST(sbjson_create_double_array_should_fail_on_allocation_failure);
    RUN_TEST(sbjson_create_string_array_should_fail_on_allocation_failure);

    RUN_TEST(sbjson_add_false_should_add_false);
    RUN_TEST(sbjson_add_false_should_fail_with_null_pointers);
    RUN_TEST(sbjson_add_false_should_fail_on_allocation_failure);

    RUN_TEST(sbjson_add_bool_should_add_bool);
    RUN_TEST(sbjson_add_bool_should_fail_with_null_pointers);
    RUN_TEST(sbjson_add_bool_should_fail_on_allocation_failure);

    RUN_TEST(sbjson_add_number_should_add_number);
    RUN_TEST(sbjson_add_number_should_fail_with_null_pointers);
    RUN_TEST(sbjson_add_number_should_fail_on_allocation_failure);

    RUN_TEST(sbjson_add_string_should_add_string);
    RUN_TEST(sbjson_add_string_should_fail_with_null_pointers);
    RUN_TEST(sbjson_add_string_should_fail_on_allocation_failure);

    RUN_TEST(sbjson_add_raw_should_add_raw);
    RUN_TEST(sbjson_add_raw_should_fail_with_null_pointers);
    RUN_TEST(sbjson_add_raw_should_fail_on_allocation_failure);

    RUN_TEST(sbJSON_add_object_should_add_object);
    RUN_TEST(sbjson_add_object_should_fail_with_null_pointers);
    RUN_TEST(sbjson_add_object_should_fail_on_allocation_failure);

    RUN_TEST(sbJSON_add_array_should_add_array);
    RUN_TEST(sbjson_add_array_should_fail_with_null_pointers);
    RUN_TEST(sbjson_add_array_should_fail_on_allocation_failure);

    return UNITY_END();
}
