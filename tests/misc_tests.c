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

static void sbjson_array_foreach_should_loop_over_arrays(void) {
    sbJSON array[1];
    sbJSON elements[10];
    sbJSON *element_pointer = NULL;
    size_t i = 0;

    memset(array, 0, sizeof(array));
    memset(elements, 0, sizeof(elements));

    /* create array */
    array[0].child = &elements[0];
    elements[0].prev = NULL;
    elements[9].next = NULL;
    for (i = 0; i < 9; i++) {
        elements[i].next = &elements[i + 1];
        elements[i + 1].prev = &elements[i];
    }

    i = 0;
    sbJSON_ArrayForEach(element_pointer, array) {
        TEST_ASSERT_TRUE_MESSAGE(element_pointer == &elements[i],
                                 "Not iterating over array properly");
        i++;
    }
}

static void sbjson_array_foreach_should_not_dereference_null_pointer(void) {
    sbJSON *array = NULL;
    sbJSON *element = NULL;
    sbJSON_ArrayForEach(element, array);
}

static void sbjson_get_object_item_should_get_object_items(void) {
    sbJSON *item = NULL;
    sbJSON *found = NULL;

    item = sbj_parse("{\"one\":1, \"two\":2, \"three\":3}");

    found = sbj_get_object_item(NULL, "test");
    TEST_ASSERT_NULL_MESSAGE(found, "Failed to fail on NULL pointer.");

    found = sbj_get_object_item(item, NULL);
    TEST_ASSERT_NULL_MESSAGE(found, "Failed to fail on NULL string.");

    found = sbj_get_object_item(item, "one");
    TEST_ASSERT_NOT_NULL_MESSAGE(found, "Failed to find first item.");
    TEST_ASSERT_FALSE(item->is_number_double);
    TEST_ASSERT_EQUAL_INT(found->u.valueint, 1);

    found = sbj_get_object_item(item, "two");
    TEST_ASSERT_NOT_NULL_MESSAGE(found, "Failed to find first item.");
    TEST_ASSERT_FALSE(item->is_number_double);
    TEST_ASSERT_EQUAL_INT(found->u.valueint, 2);

    found = sbj_get_object_item(item, "three");
    TEST_ASSERT_NOT_NULL_MESSAGE(found, "Failed to find item.");
    TEST_ASSERT_FALSE(item->is_number_double);
    TEST_ASSERT_EQUAL_INT(found->u.valueint, 3);

    found = sbj_get_object_item(item, "four");
    TEST_ASSERT_NULL_MESSAGE(found,
                             "Should not find something that isn't there.");

    sbj_delete(item);
}

static void sbjson_get_object_item_case_sensitive_should_get_object_items(void) {
    sbJSON *item = NULL;
    sbJSON *found = NULL;

    item = sbj_parse("{\"one\":1, \"Two\":2, \"tHree\":3}");

    found = sbj_get_object_item(NULL, "test");
    TEST_ASSERT_NULL_MESSAGE(found, "Failed to fail on NULL pointer.");

    found = sbj_get_object_item(item, NULL);
    TEST_ASSERT_NULL_MESSAGE(found, "Failed to fail on NULL string.");

    found = sbj_get_object_item(item, "one");
    TEST_ASSERT_NOT_NULL_MESSAGE(found, "Failed to find first item.");
    TEST_ASSERT_FALSE(item->is_number_double);
    TEST_ASSERT_EQUAL_INT(found->u.valueint, 1);

    found = sbj_get_object_item(item, "Two");
    TEST_ASSERT_NOT_NULL_MESSAGE(found, "Failed to find first item.");
    TEST_ASSERT_FALSE(item->is_number_double);
    TEST_ASSERT_EQUAL_INT(found->u.valueint, 2);

    found = sbj_get_object_item(item, "tHree");
    TEST_ASSERT_NOT_NULL_MESSAGE(found, "Failed to find item.");
    TEST_ASSERT_FALSE(item->is_number_double);
    TEST_ASSERT_EQUAL_INT(found->u.valueint, 3);

    found = sbj_get_object_item(item, "One");
    TEST_ASSERT_NULL_MESSAGE(found,
                             "Should not find something that isn't there.");

    sbj_delete(item);
}

static void sbjson_get_object_item_should_not_crash_with_array(void) {
    sbJSON *array = NULL;
    sbJSON *found = NULL;
    array = sbj_parse("[1]");

    found = sbj_get_object_item(array, "name");
    TEST_ASSERT_NULL(found);

    sbj_delete(array);
}

static void
sbjson_get_object_item_case_sensitive_should_not_crash_with_array(void) {
    sbJSON *array = NULL;
    sbJSON *found = NULL;
    array = sbj_parse("[1]");

    found = sbj_get_object_item(array, "name");
    TEST_ASSERT_NULL(found);

    sbj_delete(array);
}

static void typecheck_functions_should_check_type(void) {
    sbJSON invalid[1];
    sbJSON item[1];
    invalid->type = sbJSON_Invalid;
    invalid->string_is_const = true;
    item->type = sbJSON_Bool;
    item->u.valuebool = false;
    item->string_is_const = true;

    TEST_ASSERT_FALSE(sbj_is_invalid(NULL));
    TEST_ASSERT_FALSE(sbj_is_invalid(item));
    TEST_ASSERT_TRUE(sbj_is_invalid(invalid));

    item->type = sbJSON_Bool;
    item->u.valuebool = false;
    item->string_is_const = true;
    TEST_ASSERT_FALSE(sbj_get_bool_value(item));
    TEST_ASSERT_TRUE(sbj_is_bool(item));

    item->type = sbJSON_Bool;
    item->u.valuebool = true;
    item->string_is_const = true;
    TEST_ASSERT_TRUE(sbj_get_bool_value(item));
    TEST_ASSERT_TRUE(sbj_is_bool(item));

    item->type = sbJSON_Null;
    item->string_is_const = true;
    TEST_ASSERT_FALSE(sbj_is_null(NULL));
    TEST_ASSERT_FALSE(sbj_is_null(invalid));
    TEST_ASSERT_TRUE(sbj_is_null(item));

    item->type = sbJSON_Number;
    item->string_is_const = true;
    TEST_ASSERT_FALSE(sbj_is_number(NULL));
    TEST_ASSERT_FALSE(sbj_is_number(invalid));
    TEST_ASSERT_TRUE(sbj_is_number(item));

    item->type = sbJSON_String;
    item->string_is_const = true;
    TEST_ASSERT_FALSE(sbj_is_string(NULL));
    TEST_ASSERT_FALSE(sbj_is_string(invalid));
    TEST_ASSERT_TRUE(sbj_is_string(item));

    item->type = sbJSON_Array;
    item->string_is_const = true;
    TEST_ASSERT_FALSE(sbj_is_array(NULL));
    TEST_ASSERT_FALSE(sbj_is_array(invalid));
    TEST_ASSERT_TRUE(sbj_is_array(item));

    item->type = sbJSON_Object;
    item->string_is_const = true;
    TEST_ASSERT_FALSE(sbj_is_object(NULL));
    TEST_ASSERT_FALSE(sbj_is_object(invalid));
    TEST_ASSERT_TRUE(sbj_is_object(item));

    item->type = sbJSON_Raw;
    item->string_is_const = true;
    TEST_ASSERT_FALSE(sbj_is_raw(NULL));
    TEST_ASSERT_FALSE(sbj_is_raw(invalid));
    TEST_ASSERT_TRUE(sbj_is_raw(item));
}

static void sbjson_should_not_parse_to_deeply_nested_jsons(void) {
    char deep_json[SBJSON_NESTING_LIMIT + 1];
    size_t position = 0;

    for (position = 0; position < sizeof(deep_json); position++) {
        deep_json[position] = '[';
    }
    deep_json[sizeof(deep_json) - 1] = '\0';

    TEST_ASSERT_NULL_MESSAGE(sbj_parse(deep_json),
                             "To deep JSONs should not be parsed.");
}

static void sbjson_set_number_value_should_set_numbers(void) {
    sbJSON number[1] = {{NULL, NULL, NULL, sbJSON_Number, NULL, 0, 0, NULL}};

    sbj_set_double_number_value(number, 1.5);
    TEST_ASSERT_TRUE(number->is_number_double);
    TEST_ASSERT_EQUAL_DOUBLE(1.5, number->u.valuedouble);

    sbj_set_double_number_value(number, -1.5);
    TEST_ASSERT_TRUE(number->is_number_double);
    TEST_ASSERT_EQUAL_DOUBLE(-1.5, number->u.valuedouble);

    sbj_set_double_number_value(number, 1 + (double)INT_MAX);
    //TEST_ASSERT_EQUAL(INT_MAX, number->valueint);
    TEST_ASSERT_TRUE(number->is_number_double);
    TEST_ASSERT_EQUAL_DOUBLE(1 + (double)INT_MAX, number->u.valuedouble);

    sbj_set_double_number_value(number, -1 + (double)INT_MIN);
    //TEST_ASSERT_EQUAL(INT_MIN, number->valueint);
    TEST_ASSERT_TRUE(number->is_number_double);
    TEST_ASSERT_EQUAL_DOUBLE(-1 + (double)INT_MIN, number->u.valuedouble);
}

static void sbjson_detach_item_via_pointer_should_detach_items(void) {
    sbJSON list[4];
    sbJSON parent[1];

    memset(list, '\0', sizeof(list));

    /* link the list */
    list[0].next = &(list[1]);
    list[1].next = &(list[2]);
    list[2].next = &(list[3]);

    list[3].prev = &(list[2]);
    list[2].prev = &(list[1]);
    list[1].prev = &(list[0]);
    list[0].prev = &(list[3]);

    parent->child = &list[0];

    /* detach in the middle (list[1]) */
    TEST_ASSERT_TRUE_MESSAGE(sbj_detach_item_via_pointer(parent, &(list[1])) ==
                                 &(list[1]),
                             "Failed to detach in the middle.");
    TEST_ASSERT_TRUE_MESSAGE((list[1].prev == NULL) && (list[1].next == NULL),
                             "Didn't set pointers of detached item to NULL.");
    TEST_ASSERT_TRUE((list[0].next == &(list[2])) &&
                     (list[2].prev == &(list[0])));

    /* detach beginning (list[0]) */
    TEST_ASSERT_TRUE_MESSAGE(sbj_detach_item_via_pointer(parent, &(list[0])) ==
                                 &(list[0]),
                             "Failed to detach beginning.");
    TEST_ASSERT_TRUE_MESSAGE((list[0].prev == NULL) && (list[0].next == NULL),
                             "Didn't set pointers of detached item to NULL.");
    TEST_ASSERT_TRUE_MESSAGE((list[2].prev == &(list[3])) &&
                                 (parent->child == &(list[2])),
                             "Didn't set the new beginning.");

    /* detach end (list[3])*/
    TEST_ASSERT_TRUE_MESSAGE(sbj_detach_item_via_pointer(parent, &(list[3])) ==
                                 &(list[3]),
                             "Failed to detach end.");
    TEST_ASSERT_TRUE_MESSAGE((list[3].prev == NULL) && (list[3].next == NULL),
                             "Didn't set pointers of detached item to NULL.");
    TEST_ASSERT_TRUE_MESSAGE((list[2].next == NULL) &&
                                 (parent->child == &(list[2])),
                             "Didn't set the new end");

    /* detach single item (list[2]) */
    TEST_ASSERT_TRUE_MESSAGE(sbj_detach_item_via_pointer(parent, &list[2]) ==
                                 &list[2],
                             "Failed to detach single item.");
    TEST_ASSERT_TRUE_MESSAGE((list[2].prev == NULL) && (list[2].next == NULL),
                             "Didn't set pointers of detached item to NULL.");
    TEST_ASSERT_NULL_MESSAGE(parent->child,
                             "Child of the parent wasn't set to NULL.");
}

static void sbjson_replace_item_via_pointer_should_replace_items(void) {
    sbJSON replacements[3];
    sbJSON *beginning = NULL;
    sbJSON *middle = NULL;
    sbJSON *end = NULL;
    sbJSON *array = NULL;

    beginning = sbj_create_null();
    TEST_ASSERT_NOT_NULL(beginning);
    middle = sbj_create_null();
    TEST_ASSERT_NOT_NULL(middle);
    end = sbj_create_null();
    TEST_ASSERT_NOT_NULL(end);

    array = sbj_create_array();
    TEST_ASSERT_NOT_NULL(array);

    sbj_add_item_to_array(array, beginning);
    sbj_add_item_to_array(array, middle);
    sbj_add_item_to_array(array, end);

    memset(replacements, '\0', sizeof(replacements));

    /* replace beginning */
    TEST_ASSERT_TRUE(
        sbj_replace_item_via_pointer(array, beginning, &(replacements[0])));
    TEST_ASSERT_TRUE(replacements[0].prev == end);
    TEST_ASSERT_TRUE(replacements[0].next == middle);
    TEST_ASSERT_TRUE(middle->prev == &(replacements[0]));
    TEST_ASSERT_TRUE(array->child == &(replacements[0]));

    /* replace middle */
    TEST_ASSERT_TRUE(
        sbj_replace_item_via_pointer(array, middle, &(replacements[1])));
    TEST_ASSERT_TRUE(replacements[1].prev == &(replacements[0]));
    TEST_ASSERT_TRUE(replacements[1].next == end);
    TEST_ASSERT_TRUE(end->prev == &(replacements[1]));

    /* replace end */
    TEST_ASSERT_TRUE(
        sbj_replace_item_via_pointer(array, end, &(replacements[2])));
    TEST_ASSERT_TRUE(replacements[2].prev == &(replacements[1]));
    TEST_ASSERT_NULL(replacements[2].next);
    TEST_ASSERT_TRUE(replacements[1].next == &(replacements[2]));

    sbJSON_free(array);
}

static void sbjson_replace_item_in_object_should_preserve_name(void) {
    sbJSON root[1] = {{NULL, NULL, NULL, 0, NULL, 0, 0, NULL}};
    sbJSON *child = NULL;
    sbJSON *replacement = NULL;
    bool flag = false;

    child = sbj_create_integer_number(1);
    TEST_ASSERT_NOT_NULL(child);
    replacement = sbj_create_integer_number(2);
    TEST_ASSERT_NOT_NULL(replacement);

    flag = sbj_add_item_to_object(root, "child", child);
    TEST_ASSERT_TRUE_MESSAGE(flag, "add item to object failed");
    sbj_replace_item_in_object(root, "child", replacement);

    TEST_ASSERT_TRUE(root->child == replacement);
    TEST_ASSERT_EQUAL_STRING("child", replacement->string);

    sbj_delete(replacement);
}

static void sbjson_functions_should_not_crash_with_null_pointers(void) {
    char buffer[10];
    sbJSON *item = sbJSON_CreateString("item");
    sbJSON *array = sbj_create_array();
    sbJSON *item1 = sbJSON_CreateString("item1");
    sbJSON *item2 = sbJSON_CreateString("corrupted array item3");
    sbJSON *corruptedString = sbJSON_CreateString("corrupted");
    struct sbJSON *originalPrev;

    add_item_to_array(array, item1);
    add_item_to_array(array, item2);

    originalPrev = item2->prev;
    item2->prev = NULL;
    free(corruptedString->u.valuestring);
    corruptedString->u.valuestring = NULL;

    sbJSON_InitHooks(NULL);
    TEST_ASSERT_NULL(sbj_parse(NULL));
    TEST_ASSERT_NULL(sbj_parse_with_opts(NULL, NULL, true));
    TEST_ASSERT_NULL(sbj_print(NULL));
    TEST_ASSERT_NULL(sbj_print_unformatted(NULL));
    TEST_ASSERT_NULL(sbj_print_buffered(NULL, 10, true));
    TEST_ASSERT_FALSE(
        sbj_print_preallocated(NULL, buffer, sizeof(buffer), true));
    TEST_ASSERT_FALSE(sbj_print_preallocated(item, NULL, 1, true));
    sbj_delete(NULL);
    sbj_get_array_size(NULL);
    TEST_ASSERT_NULL(sbj_get_array_item(NULL, 0));
    TEST_ASSERT_NULL(sbj_get_object_item(NULL, "item"));
    TEST_ASSERT_NULL(sbj_get_object_item(item, NULL));
    TEST_ASSERT_NULL(sbj_get_object_item(NULL, "item"));
    TEST_ASSERT_NULL(sbj_get_object_item(item, NULL));
    TEST_ASSERT_FALSE(sbj_has_object_item(NULL, "item"));
    TEST_ASSERT_FALSE(sbj_has_object_item(item, NULL));
    TEST_ASSERT_FALSE(sbj_is_invalid(NULL));
    TEST_ASSERT_FALSE(sbj_is_bool(NULL));
    TEST_ASSERT_FALSE(sbj_is_null(NULL));
    TEST_ASSERT_FALSE(sbj_is_number(NULL));
    TEST_ASSERT_FALSE(sbj_is_string(NULL));
    TEST_ASSERT_FALSE(sbj_is_array(NULL));
    TEST_ASSERT_FALSE(sbj_is_object(NULL));
    TEST_ASSERT_FALSE(sbj_is_raw(NULL));
    TEST_ASSERT_NULL(sbJSON_CreateString(NULL));
    TEST_ASSERT_NULL(sbj_create_raw(NULL));
    TEST_ASSERT_NULL(sbj_create_int_array(NULL, 10));
    TEST_ASSERT_NULL(sbj_create_float_array(NULL, 10));
    TEST_ASSERT_NULL(sbj_create_double_array(NULL, 10));
    TEST_ASSERT_NULL(sbj_create_string_array(NULL, 10));
    sbj_add_item_to_array(NULL, item);
    sbj_add_item_to_array(item, NULL);
    sbj_add_item_to_object(item, "item", NULL);
    sbj_add_item_to_object(item, NULL, item);
    sbj_add_item_to_object(NULL, "item", item);
    sbj_add_item_to_objectCS(item, "item", NULL);
    sbj_add_item_to_objectCS(item, NULL, item);
    sbj_add_item_to_objectCS(NULL, "item", item);
    sbj_add_item_reference_to_array(NULL, item);
    sbj_add_item_reference_to_array(item, NULL);
    sbj_add_item_reference_to_object(item, "item", NULL);
    sbj_add_item_reference_to_object(item, NULL, item);
    sbj_add_item_reference_to_object(NULL, "item", item);
    TEST_ASSERT_NULL(sbj_detach_item_via_pointer(NULL, item));
    TEST_ASSERT_NULL(sbj_detach_item_via_pointer(item, NULL));
    TEST_ASSERT_NULL(sbj_detach_item_from_array(NULL, 0));
    sbj_delete_item_from_array(NULL, 0);
    TEST_ASSERT_NULL(sbj_detach_item_from_object(NULL, "item"));
    TEST_ASSERT_NULL(sbj_detach_item_from_object(item, NULL));
    TEST_ASSERT_NULL(sbj_detach_item_from_object(NULL, "item"));
    TEST_ASSERT_NULL(sbj_detach_item_from_object(item, NULL));
    sbj_delete_item_from_object(NULL, "item");
    sbj_delete_item_from_object(item, NULL);
    sbj_delete_item_from_object(NULL, "item");
    sbj_delete_item_from_object(item, NULL);
    TEST_ASSERT_FALSE(sbj_insert_item_in_array(array, 0, NULL));
    TEST_ASSERT_FALSE(sbj_insert_item_in_array(array, 1, item));
    TEST_ASSERT_FALSE(sbj_insert_item_in_array(NULL, 0, item));
    TEST_ASSERT_FALSE(sbj_insert_item_in_array(item, 0, NULL));
    TEST_ASSERT_FALSE(sbj_replace_item_via_pointer(NULL, item, item));
    TEST_ASSERT_FALSE(sbj_replace_item_via_pointer(item, NULL, item));
    TEST_ASSERT_FALSE(sbj_replace_item_via_pointer(item, item, NULL));
    TEST_ASSERT_FALSE(sbj_replace_item_in_array(item, 0, NULL));
    TEST_ASSERT_FALSE(sbj_replace_item_in_array(NULL, 0, item));
    TEST_ASSERT_FALSE(sbj_replace_item_in_object(NULL, "item", item));
    TEST_ASSERT_FALSE(sbj_replace_item_in_object(item, NULL, item));
    TEST_ASSERT_FALSE(sbj_replace_item_in_object(item, "item", NULL));
    TEST_ASSERT_FALSE(
        sbj_replace_item_in_object(NULL, "item", item));
    TEST_ASSERT_FALSE(sbj_replace_item_in_object(item, NULL, item));
    TEST_ASSERT_FALSE(
        sbj_replace_item_in_object(item, "item", NULL));
    TEST_ASSERT_NULL(sbj_duplicate(NULL, true));
    TEST_ASSERT_FALSE(sbj_compare(item, NULL));
    TEST_ASSERT_FALSE(sbj_compare(NULL, item));
    TEST_ASSERT_NULL(sbj_set_valuestring(NULL, "test"));
    TEST_ASSERT_NULL(sbj_set_valuestring(corruptedString, "test"));
    sbj_minify(NULL);
    /* skipped because it is only used via a macro that checks for NULL */
    /* sbJSON_SetNumberHelper(NULL, 0); */

    /* restore corrupted item2 to delete it */
    item2->prev = originalPrev;
    sbj_delete(corruptedString);
    sbj_delete(array);
    sbj_delete(item);
}

static void *failing_realloc(void *pointer, size_t size) {
    (void)size;
    (void)pointer;
    return NULL;
}

static void ensure_should_fail_on_failed_realloc(void) {
    printbuffer buffer = {
        NULL, 10, 0, 0, false, false, {&malloc, &free, &failing_realloc}};
    buffer.buffer = (unsigned char *)malloc(100);
    TEST_ASSERT_NOT_NULL(buffer.buffer);

    TEST_ASSERT_NULL_MESSAGE(ensure(&buffer, 200),
                             "Ensure didn't fail with failing realloc.");
}

static void skip_utf8_bom_should_skip_bom(void) {
    const unsigned char string[] = "\xEF\xBB\xBF{}";
    parse_buffer buffer = {0, 0, 0, 0, {0, 0, 0}};
    buffer.content = string;
    buffer.length = sizeof(string);
    buffer.hooks = global_hooks;

    TEST_ASSERT_TRUE(skip_utf8_bom(&buffer) == &buffer);
    TEST_ASSERT_EQUAL_UINT(3U, (unsigned int)buffer.offset);
}

static void skip_utf8_bom_should_not_skip_bom_if_not_at_beginning(void) {
    const unsigned char string[] = " \xEF\xBB\xBF{}";
    parse_buffer buffer = {0, 0, 0, 0, {0, 0, 0}};
    buffer.content = string;
    buffer.length = sizeof(string);
    buffer.hooks = global_hooks;
    buffer.offset = 1;

    TEST_ASSERT_NULL(skip_utf8_bom(&buffer));
}

static void sbjson_get_string_value_should_get_a_string(void) {
    sbJSON *string = sbJSON_CreateString("test");
    sbJSON *number = sbj_create_integer_number(1);

    TEST_ASSERT_TRUE(sbj_get_string_value(string) == string->u.valuestring);
    TEST_ASSERT_TRUE(sbj_try_get_string_value(string) == string->u.valuestring);
    //TODO: Precondition violations. Should crash.
    //TEST_ASSERT_NULL(sbj_get_string_value(number));
    //TEST_ASSERT_NULL(sbj_get_string_value(NULL));
    TEST_ASSERT_NULL(sbj_try_get_string_value(number));
    TEST_ASSERT_NULL(sbj_try_get_string_value(NULL));

    sbj_delete(number);
    sbj_delete(string);
}

static void sbjson_get_number_value_should_get_a_number(void) {
    sbJSON *string = sbJSON_CreateString("test");
    sbJSON *number = sbj_create_integer_number(1);

    TEST_ASSERT_EQUAL_DOUBLE(sbj_get_number_value(number), (double) number->u.valueint);
    //TODO: Precondition violation. This should crash.
    //TEST_ASSERT_DOUBLE_IS_NAN(sbj_get_number_value(string));
    TEST_ASSERT_DOUBLE_IS_NAN(sbj_get_number_value(NULL));

    sbj_delete(number);
    sbj_delete(string);
}

static void
sbjson_create_string_reference_should_create_a_string_reference(void) {
    const char *string = "I am a string!";

    sbJSON *string_reference = sbj_create_string_reference(string);
    TEST_ASSERT_TRUE(string_reference->u.valuestring == string);
    TEST_ASSERT_TRUE(string_reference->type == sbJSON_String);
    TEST_ASSERT_TRUE(string_reference->is_reference);
    TEST_ASSERT_FALSE(string_reference->string_is_const);

    sbj_delete(string_reference);
}

static void
sbjson_create_object_reference_should_create_an_object_reference(void) {
    sbJSON *number_reference = NULL;
    sbJSON *number_object = sbj_create_object();
    sbJSON *number = sbj_create_integer_number(42);
    const char key[] = "number";

    TEST_ASSERT_TRUE(sbj_is_number(number));
    TEST_ASSERT_TRUE(sbj_is_object(number_object));
    sbj_add_item_to_objectCS(number_object, key, number);

    number_reference = sbj_create_object_reference(number);
    TEST_ASSERT_TRUE(number_reference->child == number);
    TEST_ASSERT_TRUE(number_reference->type == sbJSON_Object);
    TEST_ASSERT_TRUE(number_reference->is_reference);
    TEST_ASSERT_FALSE(number_reference->string_is_const);

    sbj_delete(number_object);
    sbj_delete(number_reference);
}

static void
sbjson_create_array_reference_should_create_an_array_reference(void) {
    sbJSON *number_reference = NULL;
    sbJSON *number_array = sbj_create_array();
    sbJSON *number = sbj_create_integer_number(42);

    TEST_ASSERT_TRUE(sbj_is_number(number));
    TEST_ASSERT_TRUE(sbj_is_array(number_array));
    sbj_add_item_to_array(number_array, number);

    number_reference = sbj_create_array_reference(number);
    TEST_ASSERT_TRUE(number_reference->child == number);
    TEST_ASSERT_TRUE(number_reference->type == sbJSON_Array);
    TEST_ASSERT_TRUE(number_reference->is_reference);
    TEST_ASSERT_FALSE(number_reference->string_is_const);

    sbj_delete(number_array);
    sbj_delete(number_reference);
}

static void sbjson_add_item_to_object_or_array_should_not_add_itself(void) {
    sbJSON *object = sbj_create_object();
    sbJSON *array = sbj_create_array();
    bool flag = false;

    flag = sbj_add_item_to_object(object, "key", object);
    TEST_ASSERT_FALSE_MESSAGE(flag, "add an object to itself should fail");

    flag = sbj_add_item_to_array(array, array);
    TEST_ASSERT_FALSE_MESSAGE(flag, "add an array to itself should fail");

    sbj_delete(object);
    sbj_delete(array);
}

static void
sbjson_add_item_to_object_should_not_use_after_free_when_string_is_aliased(
    void) {
    sbJSON *object = sbj_create_object();
    sbJSON *number = sbj_create_integer_number(42);
    char *name =
        (char *)sbJSON_strdup((const unsigned char *)"number", &global_hooks);

    TEST_ASSERT_NOT_NULL(object);
    TEST_ASSERT_NOT_NULL(number);
    TEST_ASSERT_NOT_NULL(name);

    number->string = name;

    /* The following should not have a use after free
     * that would show up in valgrind or with AddressSanitizer */
    sbj_add_item_to_object(object, number->string, number);

    sbj_delete(object);
}

static void
sbjson_delete_item_from_array_should_not_broken_list_structure(void) {
    const char expected_json1[] = "{\"rd\":[{\"a\":\"123\"}]}";
    const char expected_json2[] = "{\"rd\":[{\"a\":\"123\"},{\"b\":\"456\"}]}";
    const char expected_json3[] = "{\"rd\":[{\"b\":\"456\"}]}";
    char *str1 = NULL;
    char *str2 = NULL;
    char *str3 = NULL;

    sbJSON *root = sbj_parse("{}");

    sbJSON *array = sbj_add_array_to_object(root, "rd");
    sbJSON *item1 = sbj_parse("{\"a\":\"123\"}");
    sbJSON *item2 = sbj_parse("{\"b\":\"456\"}");

    sbj_add_item_to_array(array, item1);
    str1 = sbj_print_unformatted(root);
    TEST_ASSERT_EQUAL_STRING(expected_json1, str1);
    free(str1);

    sbj_add_item_to_array(array, item2);
    str2 = sbj_print_unformatted(root);
    TEST_ASSERT_EQUAL_STRING(expected_json2, str2);
    free(str2);

    /* this should not broken list structure */
    sbj_delete_item_from_array(array, 0);
    str3 = sbj_print_unformatted(root);
    TEST_ASSERT_EQUAL_STRING(expected_json3, str3);
    free(str3);

    sbj_delete(root);
}

static void sbjson_set_valuestring_to_object_should_not_leak_memory(void) {
    sbJSON *root = sbj_parse("{}");
    const char *stringvalue = "valuestring could be changed safely";
    const char *reference_valuestring =
        "reference item should be freed by yourself";
    const char *short_valuestring = "shorter valuestring";
    const char *long_valuestring = "new valuestring which is much longer than "
                                   "previous should be changed safely";
    sbJSON *item1 = sbJSON_CreateString(stringvalue);
    sbJSON *item2 = sbj_create_string_reference(reference_valuestring);
    char *ptr1 = NULL;
    char *return_value = NULL;

    sbj_add_item_to_object(root, "one", item1);
    sbj_add_item_to_object(root, "two", item2);

    ptr1 = item1->u.valuestring;
    return_value = sbj_set_valuestring(sbj_get_object_item(root, "one"),
                                        short_valuestring);
    TEST_ASSERT_NOT_NULL(return_value);
    TEST_ASSERT_EQUAL_PTR_MESSAGE(
        ptr1, return_value,
        "new valuestring shorter than old should not reallocate memory");
    TEST_ASSERT_EQUAL_STRING(short_valuestring,
                             sbj_get_object_item(root, "one")->u.valuestring);

    /* we needn't to free the original valuestring manually */
    ptr1 = item1->u.valuestring;
    return_value = sbj_set_valuestring(sbj_get_object_item(root, "one"),
                                        long_valuestring);
    TEST_ASSERT_NOT_NULL(return_value);
    TEST_ASSERT_NOT_EQUAL_MESSAGE(
        ptr1, return_value,
        "new valuestring longer than old should reallocate memory");
    TEST_ASSERT_EQUAL_STRING(long_valuestring,
                             sbj_get_object_item(root, "one")->u.valuestring);

    //TODO: precondition violation; should crash
    /*return_value = sbj_set_valuestring(sbj_get_object_item(root, "two"),*/
                                        /*long_valuestring);*/
    /*TEST_ASSERT_NULL_MESSAGE(*/
        /*return_value, "valuestring of reference object should not be changed");*/
    /*TEST_ASSERT_EQUAL_STRING(reference_valuestring,*/
                             /*sbj_get_object_item(root, "two")->u.valuestring);*/

    sbj_delete(root);
}

static void sbjson_set_bool_value_must_not_break_objects(void) {
    sbJSON *bobj, *sobj, *oobj, *refobj = NULL;

    TEST_ASSERT_TRUE((sbj_set_bool_value(refobj, 1) == sbJSON_Invalid));

    bobj = sbj_create_bool(false);
    TEST_ASSERT_TRUE(!sbj_get_bool_value(bobj));
    TEST_ASSERT_TRUE((sbj_set_bool_value(bobj, 1)));
    TEST_ASSERT_TRUE(sbj_get_bool_value(bobj));
    sbj_set_bool_value(bobj, 1);
    TEST_ASSERT_TRUE(sbj_get_bool_value(bobj));
    TEST_ASSERT_TRUE((sbj_set_bool_value(bobj, 0)));
    TEST_ASSERT_TRUE(!sbj_get_bool_value(bobj));
    sbj_set_bool_value(bobj, 0);
    TEST_ASSERT_TRUE(!sbj_get_bool_value(bobj));

    sobj = sbJSON_CreateString("test");
    TEST_ASSERT_TRUE(sbj_is_string(sobj));
    sbj_set_bool_value(sobj, 1);
    TEST_ASSERT_TRUE(sbj_is_string(sobj));
    sbj_set_bool_value(sobj, 0);
    TEST_ASSERT_TRUE(sbj_is_string(sobj));

    oobj = sbj_create_object();
    TEST_ASSERT_TRUE(sbj_is_object(oobj));
    sbj_set_bool_value(oobj, 1);
    TEST_ASSERT_TRUE(sbj_is_object(oobj));
    sbj_set_bool_value(oobj, 0);
    TEST_ASSERT_TRUE(sbj_is_object(oobj));

    refobj = sbj_create_string_reference("conststring");
    TEST_ASSERT_TRUE(sbj_is_string(refobj));
    TEST_ASSERT_TRUE(refobj->is_reference);
    sbj_set_bool_value(refobj, 1);
    TEST_ASSERT_TRUE(sbj_is_string(refobj));
    TEST_ASSERT_TRUE(refobj->is_reference);
    sbj_set_bool_value(refobj, 0);
    TEST_ASSERT_TRUE(sbj_is_string(refobj));
    TEST_ASSERT_TRUE(refobj->is_reference);
    sbj_delete(refobj);

    refobj = sbj_create_object_reference(oobj);
    TEST_ASSERT_TRUE(sbj_is_object(refobj));
    TEST_ASSERT_TRUE(refobj->is_reference);
    sbj_set_bool_value(refobj, 1);
    TEST_ASSERT_TRUE(sbj_is_object(refobj));
    TEST_ASSERT_TRUE(refobj->is_reference);
    sbj_set_bool_value(refobj, 0);
    TEST_ASSERT_TRUE(sbj_is_object(refobj));
    TEST_ASSERT_TRUE(refobj->is_reference);
    sbj_delete(refobj);

    sbj_delete(oobj);
    sbj_delete(bobj);
    sbj_delete(sobj);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(sbjson_array_foreach_should_loop_over_arrays);
    RUN_TEST(sbjson_array_foreach_should_not_dereference_null_pointer);
    RUN_TEST(sbjson_get_object_item_should_get_object_items);
    RUN_TEST(sbjson_get_object_item_case_sensitive_should_get_object_items);
    RUN_TEST(sbjson_get_object_item_should_not_crash_with_array);
    RUN_TEST(sbjson_get_object_item_case_sensitive_should_not_crash_with_array);
    RUN_TEST(typecheck_functions_should_check_type);
    RUN_TEST(sbjson_should_not_parse_to_deeply_nested_jsons);
    RUN_TEST(sbjson_set_number_value_should_set_numbers);
    RUN_TEST(sbjson_detach_item_via_pointer_should_detach_items);
    RUN_TEST(sbjson_replace_item_via_pointer_should_replace_items);
    RUN_TEST(sbjson_replace_item_in_object_should_preserve_name);
    //TODO: Should crash for broken preconditions, but not for other null pointers.
    //RUN_TEST(sbjson_functions_should_not_crash_with_null_pointers);
    RUN_TEST(ensure_should_fail_on_failed_realloc);
    RUN_TEST(skip_utf8_bom_should_skip_bom);
    RUN_TEST(skip_utf8_bom_should_not_skip_bom_if_not_at_beginning);
    RUN_TEST(sbjson_get_string_value_should_get_a_string);
    RUN_TEST(sbjson_get_number_value_should_get_a_number);
    RUN_TEST(sbjson_create_string_reference_should_create_a_string_reference);
    RUN_TEST(sbjson_create_object_reference_should_create_an_object_reference);
    RUN_TEST(sbjson_create_array_reference_should_create_an_array_reference);
    //TODO: Precondition violation; should crash (in debug)
    //RUN_TEST(sbjson_add_item_to_object_or_array_should_not_add_itself);
    RUN_TEST(
        sbjson_add_item_to_object_should_not_use_after_free_when_string_is_aliased);
    RUN_TEST(sbjson_delete_item_from_array_should_not_broken_list_structure);
    RUN_TEST(sbjson_set_valuestring_to_object_should_not_leak_memory);
    RUN_TEST(sbjson_set_bool_value_must_not_break_objects);

    return UNITY_END();
}
