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
    sbJSON *root = sbj_create_object();
    sbJSON *null = NULL;

    sbj_add_null_to_object(root, "null");

    TEST_ASSERT_NOT_NULL(null = sbj_get_object_item(root, "null"));
    TEST_ASSERT_EQUAL_INT(null->type, sbJSON_Null);

    sbj_delete(root);
}

static void sbjson_add_null_should_fail_with_null_pointers(void) {
    sbJSON *root = sbj_create_object();

    TEST_ASSERT_NULL(sbj_add_null_to_object(NULL, "null"));
    TEST_ASSERT_NULL(sbj_add_null_to_object(root, NULL));

    sbj_delete(root);
}

static void sbjson_add_null_should_fail_on_allocation_failure(void) {
    sbJSON *root = sbj_create_object();

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbj_add_null_to_object(root, "null"));

    sbJSON_InitHooks(NULL);

    sbj_delete(root);
}

static void sbjson_add_true_should_add_true(void) {
    sbJSON *root = sbj_create_object();
    sbJSON *true_item = NULL;

    sbj_add_true_to_object(root, "true");

    TEST_ASSERT_NOT_NULL(true_item = sbj_get_object_item(root, "true"));
    TEST_ASSERT_EQUAL_INT(true_item->type, sbJSON_Bool);
    TEST_ASSERT_EQUAL(true_item->u.valuebool, true);

    sbj_delete(root);
}

static void sbjson_add_true_should_fail_with_null_pointers(void) {
    sbJSON *root = sbj_create_object();

    TEST_ASSERT_NULL(sbj_add_true_to_object(NULL, "true"));
    TEST_ASSERT_NULL(sbj_add_true_to_object(root, NULL));

    sbj_delete(root);
}

static void sbjson_add_true_should_fail_on_allocation_failure(void) {
    sbJSON *root = sbj_create_object();

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbj_add_true_to_object(root, "true"));

    sbJSON_InitHooks(NULL);

    sbj_delete(root);
}

static void sbjson_create_int_array_should_fail_on_allocation_failure(void) {
    int numbers[] = {1, 2, 3};

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbj_create_int_array(numbers, 3));

    sbJSON_InitHooks(NULL);
}

static void sbjson_create_float_array_should_fail_on_allocation_failure(void) {
    float numbers[] = {1.0f, 2.0f, 3.0f};

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbj_create_float_array(numbers, 3));

    sbJSON_InitHooks(NULL);
}

static void sbjson_create_double_array_should_fail_on_allocation_failure(void) {
    double numbers[] = {1.0, 2.0, 3.0};

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbj_create_double_array(numbers, 3));

    sbJSON_InitHooks(NULL);
}

static void sbjson_create_string_array_should_fail_on_allocation_failure(void) {
    const char *strings[] = {"1", "2", "3"};

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbj_create_string_array(strings, 3));

    sbJSON_InitHooks(NULL);
}

static void sbjson_add_false_should_add_false(void) {
    sbJSON *root = sbj_create_object();
    sbJSON *false_item = NULL;

    sbj_add_false_to_object(root, "false");

    TEST_ASSERT_NOT_NULL(false_item = sbj_get_object_item(root, "false"));
    TEST_ASSERT_EQUAL_INT(false_item->type, sbJSON_Bool);
    TEST_ASSERT_EQUAL(false_item->u.valuebool, false);

    sbj_delete(root);
}

static void sbjson_add_false_should_fail_with_null_pointers(void) {
    sbJSON *root = sbj_create_object();

    TEST_ASSERT_NULL(sbj_add_false_to_object(NULL, "false"));
    TEST_ASSERT_NULL(sbj_add_false_to_object(root, NULL));

    sbj_delete(root);
}

static void sbjson_add_false_should_fail_on_allocation_failure(void) {
    sbJSON *root = sbj_create_object();

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbj_add_false_to_object(root, "false"));

    sbJSON_InitHooks(NULL);

    sbj_delete(root);
}

static void sbjson_add_bool_should_add_bool(void) {
    sbJSON *root = sbj_create_object();
    sbJSON *true_item = NULL;
    sbJSON *false_item = NULL;

    /* true */
    sbj_add_bool_to_object(root, "true", true);
    TEST_ASSERT_NOT_NULL(true_item = sbj_get_object_item(root, "true"));
    TEST_ASSERT_EQUAL_INT(true_item->type, sbJSON_Bool);
    TEST_ASSERT_EQUAL(true_item->u.valuebool, true);

    /* false */
    sbj_add_bool_to_object(root, "false", false);
    TEST_ASSERT_NOT_NULL(false_item = sbj_get_object_item(root, "false"));
    TEST_ASSERT_EQUAL_INT(false_item->type, sbJSON_Bool);
    TEST_ASSERT_EQUAL(false_item->u.valuebool, false);

    sbj_delete(root);
}

static void sbjson_add_bool_should_fail_with_null_pointers(void) {
    sbJSON *root = sbj_create_object();

    TEST_ASSERT_NULL(sbj_add_bool_to_object(NULL, "false", false));
    TEST_ASSERT_NULL(sbj_add_bool_to_object(root, NULL, false));

    sbj_delete(root);
}

static void sbjson_add_bool_should_fail_on_allocation_failure(void) {
    sbJSON *root = sbj_create_object();

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbj_add_bool_to_object(root, "false", false));

    sbJSON_InitHooks(NULL);

    sbj_delete(root);
}

static void sbjson_add_number_should_add_number(void) {
    sbJSON *root = sbj_create_object();
    sbJSON *number = NULL;

    sbj_add_integer_number_to_object(root, "number", 42);

    TEST_ASSERT_NOT_NULL(number = sbj_get_object_item(root, "number"));

    TEST_ASSERT_EQUAL_INT(number->type, sbJSON_Number);
    TEST_ASSERT_FALSE(number->is_number_double);
    TEST_ASSERT_EQUAL_INT(number->u.valueint, 42);

    sbj_delete(root);
}

static void sbjson_add_number_should_fail_with_null_pointers(void) {
    sbJSON *root = sbj_create_object();

    TEST_ASSERT_NULL(sbj_add_integer_number_to_object(NULL, "number", 42));
    TEST_ASSERT_NULL(sbj_add_integer_number_to_object(root, NULL, 42));

    sbj_delete(root);
}

static void sbjson_add_number_should_fail_on_allocation_failure(void) {
    sbJSON *root = sbj_create_object();

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbj_add_integer_number_to_object(root, "number", 42));

    sbJSON_InitHooks(NULL);

    sbj_delete(root);
}

static void sbjson_add_string_should_add_string(void) {
    sbJSON *root = sbj_create_object();
    sbJSON *string = NULL;

    sbj_add_string_to_object(root, "string", "Hello World!");

    TEST_ASSERT_NOT_NULL(string = sbj_get_object_item(root, "string"));
    TEST_ASSERT_EQUAL_INT(string->type, sbJSON_String);
    TEST_ASSERT_EQUAL_STRING(string->u.valuestring, "Hello World!");

    sbj_delete(root);
}

static void sbjson_add_string_should_fail_with_null_pointers(void) {
    sbJSON *root = sbj_create_object();

    TEST_ASSERT_NULL(sbj_add_string_to_object(NULL, "string", "string"));
    TEST_ASSERT_NULL(sbj_add_string_to_object(root, NULL, "string"));

    sbj_delete(root);
}

static void sbjson_add_string_should_fail_on_allocation_failure(void) {
    sbJSON *root = sbj_create_object();

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbj_add_string_to_object(root, "string", "string"));

    sbJSON_InitHooks(NULL);

    sbj_delete(root);
}

static void sbjson_add_raw_should_add_raw(void) {
    sbJSON *root = sbj_create_object();
    sbJSON *raw = NULL;

    sbj_add_raw_to_object(root, "raw", "{}");

    TEST_ASSERT_NOT_NULL(raw = sbj_get_object_item(root, "raw"));
    TEST_ASSERT_EQUAL_INT(raw->type, sbJSON_Raw);
    TEST_ASSERT_EQUAL_STRING(raw->u.valuestring, "{}");

    sbj_delete(root);
}

static void sbjson_add_raw_should_fail_with_null_pointers(void) {
    sbJSON *root = sbj_create_object();

    TEST_ASSERT_NULL(sbj_add_raw_to_object(NULL, "raw", "{}"));
    TEST_ASSERT_NULL(sbj_add_raw_to_object(root, NULL, "{}"));

    sbj_delete(root);
}

static void sbjson_add_raw_should_fail_on_allocation_failure(void) {
    sbJSON *root = sbj_create_object();

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbj_add_raw_to_object(root, "raw", "{}"));

    sbJSON_InitHooks(NULL);

    sbj_delete(root);
}

static void sbJSON_add_object_should_add_object(void) {
    sbJSON *root = sbj_create_object();
    sbJSON *object = NULL;

    sbj_add_object_to_object(root, "object");
    TEST_ASSERT_NOT_NULL(object = sbj_get_object_item(root, "object"));
    TEST_ASSERT_EQUAL_INT(object->type, sbJSON_Object);

    sbj_delete(root);
}

static void sbjson_add_object_should_fail_with_null_pointers(void) {
    sbJSON *root = sbj_create_object();

    TEST_ASSERT_NULL(sbj_add_object_to_object(NULL, "object"));
    TEST_ASSERT_NULL(sbj_add_object_to_object(root, NULL));

    sbj_delete(root);
}

static void sbjson_add_object_should_fail_on_allocation_failure(void) {
    sbJSON *root = sbj_create_object();

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbj_add_object_to_object(root, "object"));

    sbJSON_InitHooks(NULL);

    sbj_delete(root);
}

static void sbJSON_add_array_should_add_array(void) {
    sbJSON *root = sbj_create_object();
    sbJSON *array = NULL;

    sbj_add_array_to_object(root, "array");
    TEST_ASSERT_NOT_NULL(array = sbj_get_object_item(root, "array"));
    TEST_ASSERT_EQUAL_INT(array->type, sbJSON_Array);

    sbj_delete(root);
}

static void sbjson_add_array_should_fail_with_null_pointers(void) {
    sbJSON *root = sbj_create_object();

    TEST_ASSERT_NULL(sbj_add_array_to_object(NULL, "array"));
    TEST_ASSERT_NULL(sbj_add_array_to_object(root, NULL));

    sbj_delete(root);
}

static void sbjson_add_array_should_fail_on_allocation_failure(void) {
    sbJSON *root = sbj_create_object();

    sbJSON_InitHooks(&failing_hooks);

    TEST_ASSERT_NULL(sbj_add_array_to_object(root, "array"));

    sbJSON_InitHooks(NULL);

    sbj_delete(root);
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
