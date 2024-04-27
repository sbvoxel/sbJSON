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

    item = sbJSON_Parse("{\"one\":1, \"Two\":2, \"tHree\":3}");

    found = sbJSON_GetObjectItem(NULL, "test");
    TEST_ASSERT_NULL_MESSAGE(found, "Failed to fail on NULL pointer.");

    found = sbJSON_GetObjectItem(item, NULL);
    TEST_ASSERT_NULL_MESSAGE(found, "Failed to fail on NULL string.");

    found = sbJSON_GetObjectItem(item, "one");
    TEST_ASSERT_NOT_NULL_MESSAGE(found, "Failed to find first item.");
    TEST_ASSERT_FALSE(item->is_number_double);
    TEST_ASSERT_EQUAL_INT(found->u.valueint, 1);

    found = sbJSON_GetObjectItem(item, "tWo");
    TEST_ASSERT_NOT_NULL_MESSAGE(found, "Failed to find first item.");
    TEST_ASSERT_FALSE(item->is_number_double);
    TEST_ASSERT_EQUAL_INT(found->u.valueint, 2);

    found = sbJSON_GetObjectItem(item, "three");
    TEST_ASSERT_NOT_NULL_MESSAGE(found, "Failed to find item.");
    TEST_ASSERT_FALSE(item->is_number_double);
    TEST_ASSERT_EQUAL_INT(found->u.valueint, 3);

    found = sbJSON_GetObjectItem(item, "four");
    TEST_ASSERT_NULL_MESSAGE(found,
                             "Should not find something that isn't there.");

    sbJSON_Delete(item);
}

static void sbjson_get_object_item_case_sensitive_should_get_object_items(void) {
    sbJSON *item = NULL;
    sbJSON *found = NULL;

    item = sbJSON_Parse("{\"one\":1, \"Two\":2, \"tHree\":3}");

    found = sbJSON_GetObjectItemCaseSensitive(NULL, "test");
    TEST_ASSERT_NULL_MESSAGE(found, "Failed to fail on NULL pointer.");

    found = sbJSON_GetObjectItemCaseSensitive(item, NULL);
    TEST_ASSERT_NULL_MESSAGE(found, "Failed to fail on NULL string.");

    found = sbJSON_GetObjectItemCaseSensitive(item, "one");
    TEST_ASSERT_NOT_NULL_MESSAGE(found, "Failed to find first item.");
    TEST_ASSERT_FALSE(item->is_number_double);
    TEST_ASSERT_EQUAL_INT(found->u.valueint, 1);

    found = sbJSON_GetObjectItemCaseSensitive(item, "Two");
    TEST_ASSERT_NOT_NULL_MESSAGE(found, "Failed to find first item.");
    TEST_ASSERT_FALSE(item->is_number_double);
    TEST_ASSERT_EQUAL_INT(found->u.valueint, 2);

    found = sbJSON_GetObjectItemCaseSensitive(item, "tHree");
    TEST_ASSERT_NOT_NULL_MESSAGE(found, "Failed to find item.");
    TEST_ASSERT_FALSE(item->is_number_double);
    TEST_ASSERT_EQUAL_INT(found->u.valueint, 3);

    found = sbJSON_GetObjectItemCaseSensitive(item, "One");
    TEST_ASSERT_NULL_MESSAGE(found,
                             "Should not find something that isn't there.");

    sbJSON_Delete(item);
}

static void sbjson_get_object_item_should_not_crash_with_array(void) {
    sbJSON *array = NULL;
    sbJSON *found = NULL;
    array = sbJSON_Parse("[1]");

    found = sbJSON_GetObjectItem(array, "name");
    TEST_ASSERT_NULL(found);

    sbJSON_Delete(array);
}

static void
sbjson_get_object_item_case_sensitive_should_not_crash_with_array(void) {
    sbJSON *array = NULL;
    sbJSON *found = NULL;
    array = sbJSON_Parse("[1]");

    found = sbJSON_GetObjectItemCaseSensitive(array, "name");
    TEST_ASSERT_NULL(found);

    sbJSON_Delete(array);
}

static void typecheck_functions_should_check_type(void) {
    sbJSON invalid[1];
    sbJSON item[1];
    invalid->type = sbJSON_Invalid;
    invalid->string_is_const = true;
    item->type = sbJSON_False;
    item->string_is_const = true;

    TEST_ASSERT_FALSE(sbJSON_IsInvalid(NULL));
    TEST_ASSERT_FALSE(sbJSON_IsInvalid(item));
    TEST_ASSERT_TRUE(sbJSON_IsInvalid(invalid));

    item->type = sbJSON_False;
    item->string_is_const = true;
    TEST_ASSERT_FALSE(sbJSON_IsFalse(NULL));
    TEST_ASSERT_FALSE(sbJSON_IsFalse(invalid));
    TEST_ASSERT_TRUE(sbJSON_IsFalse(item));
    TEST_ASSERT_TRUE(sbJSON_IsBool(item));

    item->type = sbJSON_True;
    item->string_is_const = true;
    TEST_ASSERT_FALSE(sbJSON_IsTrue(NULL));
    TEST_ASSERT_FALSE(sbJSON_IsTrue(invalid));
    TEST_ASSERT_TRUE(sbJSON_IsTrue(item));
    TEST_ASSERT_TRUE(sbJSON_IsBool(item));

    item->type = sbJSON_Null;
    item->string_is_const = true;
    TEST_ASSERT_FALSE(sbJSON_IsNull(NULL));
    TEST_ASSERT_FALSE(sbJSON_IsNull(invalid));
    TEST_ASSERT_TRUE(sbJSON_IsNull(item));

    item->type = sbJSON_Number;
    item->string_is_const = true;
    TEST_ASSERT_FALSE(sbJSON_IsNumber(NULL));
    TEST_ASSERT_FALSE(sbJSON_IsNumber(invalid));
    TEST_ASSERT_TRUE(sbJSON_IsNumber(item));

    item->type = sbJSON_String;
    item->string_is_const = true;
    TEST_ASSERT_FALSE(sbJSON_IsString(NULL));
    TEST_ASSERT_FALSE(sbJSON_IsString(invalid));
    TEST_ASSERT_TRUE(sbJSON_IsString(item));

    item->type = sbJSON_Array;
    item->string_is_const = true;
    TEST_ASSERT_FALSE(sbJSON_IsArray(NULL));
    TEST_ASSERT_FALSE(sbJSON_IsArray(invalid));
    TEST_ASSERT_TRUE(sbJSON_IsArray(item));

    item->type = sbJSON_Object;
    item->string_is_const = true;
    TEST_ASSERT_FALSE(sbJSON_IsObject(NULL));
    TEST_ASSERT_FALSE(sbJSON_IsObject(invalid));
    TEST_ASSERT_TRUE(sbJSON_IsObject(item));

    item->type = sbJSON_Raw;
    item->string_is_const = true;
    TEST_ASSERT_FALSE(sbJSON_IsRaw(NULL));
    TEST_ASSERT_FALSE(sbJSON_IsRaw(invalid));
    TEST_ASSERT_TRUE(sbJSON_IsRaw(item));
}

static void sbjson_should_not_parse_to_deeply_nested_jsons(void) {
    char deep_json[SBJSON_NESTING_LIMIT + 1];
    size_t position = 0;

    for (position = 0; position < sizeof(deep_json); position++) {
        deep_json[position] = '[';
    }
    deep_json[sizeof(deep_json) - 1] = '\0';

    TEST_ASSERT_NULL_MESSAGE(sbJSON_Parse(deep_json),
                             "To deep JSONs should not be parsed.");
}

static void sbjson_set_number_value_should_set_numbers(void) {
    sbJSON number[1] = {{NULL, NULL, NULL, sbJSON_Number, NULL, 0, 0, NULL}};

    sbJSON_SetDoubleNumberValue(number, 1.5);
    TEST_ASSERT_TRUE(number->is_number_double);
    TEST_ASSERT_EQUAL_DOUBLE(1.5, number->u.valuedouble);

    sbJSON_SetDoubleNumberValue(number, -1.5);
    TEST_ASSERT_TRUE(number->is_number_double);
    TEST_ASSERT_EQUAL_DOUBLE(-1.5, number->u.valuedouble);

    sbJSON_SetDoubleNumberValue(number, 1 + (double)INT_MAX);
    //TEST_ASSERT_EQUAL(INT_MAX, number->valueint);
    TEST_ASSERT_TRUE(number->is_number_double);
    TEST_ASSERT_EQUAL_DOUBLE(1 + (double)INT_MAX, number->u.valuedouble);

    sbJSON_SetDoubleNumberValue(number, -1 + (double)INT_MIN);
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
    TEST_ASSERT_TRUE_MESSAGE(sbJSON_DetachItemViaPointer(parent, &(list[1])) ==
                                 &(list[1]),
                             "Failed to detach in the middle.");
    TEST_ASSERT_TRUE_MESSAGE((list[1].prev == NULL) && (list[1].next == NULL),
                             "Didn't set pointers of detached item to NULL.");
    TEST_ASSERT_TRUE((list[0].next == &(list[2])) &&
                     (list[2].prev == &(list[0])));

    /* detach beginning (list[0]) */
    TEST_ASSERT_TRUE_MESSAGE(sbJSON_DetachItemViaPointer(parent, &(list[0])) ==
                                 &(list[0]),
                             "Failed to detach beginning.");
    TEST_ASSERT_TRUE_MESSAGE((list[0].prev == NULL) && (list[0].next == NULL),
                             "Didn't set pointers of detached item to NULL.");
    TEST_ASSERT_TRUE_MESSAGE((list[2].prev == &(list[3])) &&
                                 (parent->child == &(list[2])),
                             "Didn't set the new beginning.");

    /* detach end (list[3])*/
    TEST_ASSERT_TRUE_MESSAGE(sbJSON_DetachItemViaPointer(parent, &(list[3])) ==
                                 &(list[3]),
                             "Failed to detach end.");
    TEST_ASSERT_TRUE_MESSAGE((list[3].prev == NULL) && (list[3].next == NULL),
                             "Didn't set pointers of detached item to NULL.");
    TEST_ASSERT_TRUE_MESSAGE((list[2].next == NULL) &&
                                 (parent->child == &(list[2])),
                             "Didn't set the new end");

    /* detach single item (list[2]) */
    TEST_ASSERT_TRUE_MESSAGE(sbJSON_DetachItemViaPointer(parent, &list[2]) ==
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

    beginning = sbJSON_CreateNull();
    TEST_ASSERT_NOT_NULL(beginning);
    middle = sbJSON_CreateNull();
    TEST_ASSERT_NOT_NULL(middle);
    end = sbJSON_CreateNull();
    TEST_ASSERT_NOT_NULL(end);

    array = sbJSON_CreateArray();
    TEST_ASSERT_NOT_NULL(array);

    sbJSON_AddItemToArray(array, beginning);
    sbJSON_AddItemToArray(array, middle);
    sbJSON_AddItemToArray(array, end);

    memset(replacements, '\0', sizeof(replacements));

    /* replace beginning */
    TEST_ASSERT_TRUE(
        sbJSON_ReplaceItemViaPointer(array, beginning, &(replacements[0])));
    TEST_ASSERT_TRUE(replacements[0].prev == end);
    TEST_ASSERT_TRUE(replacements[0].next == middle);
    TEST_ASSERT_TRUE(middle->prev == &(replacements[0]));
    TEST_ASSERT_TRUE(array->child == &(replacements[0]));

    /* replace middle */
    TEST_ASSERT_TRUE(
        sbJSON_ReplaceItemViaPointer(array, middle, &(replacements[1])));
    TEST_ASSERT_TRUE(replacements[1].prev == &(replacements[0]));
    TEST_ASSERT_TRUE(replacements[1].next == end);
    TEST_ASSERT_TRUE(end->prev == &(replacements[1]));

    /* replace end */
    TEST_ASSERT_TRUE(
        sbJSON_ReplaceItemViaPointer(array, end, &(replacements[2])));
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

    child = sbJSON_CreateIntegerNumber(1);
    TEST_ASSERT_NOT_NULL(child);
    replacement = sbJSON_CreateIntegerNumber(2);
    TEST_ASSERT_NOT_NULL(replacement);

    flag = sbJSON_AddItemToObject(root, "child", child);
    TEST_ASSERT_TRUE_MESSAGE(flag, "add item to object failed");
    sbJSON_ReplaceItemInObject(root, "child", replacement);

    TEST_ASSERT_TRUE(root->child == replacement);
    TEST_ASSERT_EQUAL_STRING("child", replacement->string);

    sbJSON_Delete(replacement);
}

static void sbjson_functions_should_not_crash_with_null_pointers(void) {
    char buffer[10];
    sbJSON *item = sbJSON_CreateString("item");
    sbJSON *array = sbJSON_CreateArray();
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
    TEST_ASSERT_NULL(sbJSON_Parse(NULL));
    TEST_ASSERT_NULL(sbJSON_ParseWithOpts(NULL, NULL, true));
    TEST_ASSERT_NULL(sbJSON_Print(NULL));
    TEST_ASSERT_NULL(sbJSON_PrintUnformatted(NULL));
    TEST_ASSERT_NULL(sbJSON_PrintBuffered(NULL, 10, true));
    TEST_ASSERT_FALSE(
        sbJSON_PrintPreallocated(NULL, buffer, sizeof(buffer), true));
    TEST_ASSERT_FALSE(sbJSON_PrintPreallocated(item, NULL, 1, true));
    sbJSON_Delete(NULL);
    sbJSON_GetArraySize(NULL);
    TEST_ASSERT_NULL(sbJSON_GetArrayItem(NULL, 0));
    TEST_ASSERT_NULL(sbJSON_GetObjectItem(NULL, "item"));
    TEST_ASSERT_NULL(sbJSON_GetObjectItem(item, NULL));
    TEST_ASSERT_NULL(sbJSON_GetObjectItemCaseSensitive(NULL, "item"));
    TEST_ASSERT_NULL(sbJSON_GetObjectItemCaseSensitive(item, NULL));
    TEST_ASSERT_FALSE(sbJSON_HasObjectItem(NULL, "item"));
    TEST_ASSERT_FALSE(sbJSON_HasObjectItem(item, NULL));
    TEST_ASSERT_FALSE(sbJSON_IsInvalid(NULL));
    TEST_ASSERT_FALSE(sbJSON_IsFalse(NULL));
    TEST_ASSERT_FALSE(sbJSON_IsTrue(NULL));
    TEST_ASSERT_FALSE(sbJSON_IsBool(NULL));
    TEST_ASSERT_FALSE(sbJSON_IsNull(NULL));
    TEST_ASSERT_FALSE(sbJSON_IsNumber(NULL));
    TEST_ASSERT_FALSE(sbJSON_IsString(NULL));
    TEST_ASSERT_FALSE(sbJSON_IsArray(NULL));
    TEST_ASSERT_FALSE(sbJSON_IsObject(NULL));
    TEST_ASSERT_FALSE(sbJSON_IsRaw(NULL));
    TEST_ASSERT_NULL(sbJSON_CreateString(NULL));
    TEST_ASSERT_NULL(sbJSON_CreateRaw(NULL));
    TEST_ASSERT_NULL(sbJSON_CreateIntArray(NULL, 10));
    TEST_ASSERT_NULL(sbJSON_CreateFloatArray(NULL, 10));
    TEST_ASSERT_NULL(sbJSON_CreateDoubleArray(NULL, 10));
    TEST_ASSERT_NULL(sbJSON_CreateStringArray(NULL, 10));
    sbJSON_AddItemToArray(NULL, item);
    sbJSON_AddItemToArray(item, NULL);
    sbJSON_AddItemToObject(item, "item", NULL);
    sbJSON_AddItemToObject(item, NULL, item);
    sbJSON_AddItemToObject(NULL, "item", item);
    sbJSON_AddItemToObjectCS(item, "item", NULL);
    sbJSON_AddItemToObjectCS(item, NULL, item);
    sbJSON_AddItemToObjectCS(NULL, "item", item);
    sbJSON_AddItemReferenceToArray(NULL, item);
    sbJSON_AddItemReferenceToArray(item, NULL);
    sbJSON_AddItemReferenceToObject(item, "item", NULL);
    sbJSON_AddItemReferenceToObject(item, NULL, item);
    sbJSON_AddItemReferenceToObject(NULL, "item", item);
    TEST_ASSERT_NULL(sbJSON_DetachItemViaPointer(NULL, item));
    TEST_ASSERT_NULL(sbJSON_DetachItemViaPointer(item, NULL));
    TEST_ASSERT_NULL(sbJSON_DetachItemFromArray(NULL, 0));
    sbJSON_DeleteItemFromArray(NULL, 0);
    TEST_ASSERT_NULL(sbJSON_DetachItemFromObject(NULL, "item"));
    TEST_ASSERT_NULL(sbJSON_DetachItemFromObject(item, NULL));
    TEST_ASSERT_NULL(sbJSON_DetachItemFromObjectCaseSensitive(NULL, "item"));
    TEST_ASSERT_NULL(sbJSON_DetachItemFromObjectCaseSensitive(item, NULL));
    sbJSON_DeleteItemFromObject(NULL, "item");
    sbJSON_DeleteItemFromObject(item, NULL);
    sbJSON_DeleteItemFromObjectCaseSensitive(NULL, "item");
    sbJSON_DeleteItemFromObjectCaseSensitive(item, NULL);
    TEST_ASSERT_FALSE(sbJSON_InsertItemInArray(array, 0, NULL));
    TEST_ASSERT_FALSE(sbJSON_InsertItemInArray(array, 1, item));
    TEST_ASSERT_FALSE(sbJSON_InsertItemInArray(NULL, 0, item));
    TEST_ASSERT_FALSE(sbJSON_InsertItemInArray(item, 0, NULL));
    TEST_ASSERT_FALSE(sbJSON_ReplaceItemViaPointer(NULL, item, item));
    TEST_ASSERT_FALSE(sbJSON_ReplaceItemViaPointer(item, NULL, item));
    TEST_ASSERT_FALSE(sbJSON_ReplaceItemViaPointer(item, item, NULL));
    TEST_ASSERT_FALSE(sbJSON_ReplaceItemInArray(item, 0, NULL));
    TEST_ASSERT_FALSE(sbJSON_ReplaceItemInArray(NULL, 0, item));
    TEST_ASSERT_FALSE(sbJSON_ReplaceItemInObject(NULL, "item", item));
    TEST_ASSERT_FALSE(sbJSON_ReplaceItemInObject(item, NULL, item));
    TEST_ASSERT_FALSE(sbJSON_ReplaceItemInObject(item, "item", NULL));
    TEST_ASSERT_FALSE(
        sbJSON_ReplaceItemInObjectCaseSensitive(NULL, "item", item));
    TEST_ASSERT_FALSE(sbJSON_ReplaceItemInObjectCaseSensitive(item, NULL, item));
    TEST_ASSERT_FALSE(
        sbJSON_ReplaceItemInObjectCaseSensitive(item, "item", NULL));
    TEST_ASSERT_NULL(sbJSON_Duplicate(NULL, true));
    TEST_ASSERT_FALSE(sbJSON_Compare(item, NULL, false));
    TEST_ASSERT_FALSE(sbJSON_Compare(NULL, item, false));
    TEST_ASSERT_NULL(sbJSON_SetValuestring(NULL, "test"));
    TEST_ASSERT_NULL(sbJSON_SetValuestring(corruptedString, "test"));
    sbJSON_Minify(NULL);
    /* skipped because it is only used via a macro that checks for NULL */
    /* sbJSON_SetNumberHelper(NULL, 0); */

    /* restore corrupted item2 to delete it */
    item2->prev = originalPrev;
    sbJSON_Delete(corruptedString);
    sbJSON_Delete(array);
    sbJSON_Delete(item);
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
    sbJSON *number = sbJSON_CreateIntegerNumber(1);

    TEST_ASSERT_TRUE(sbJSON_GetStringValue(string) == string->u.valuestring);
    TEST_ASSERT_TRUE(sbJSON_TryGetStringValue(string) == string->u.valuestring);
    //TODO: Precondition violations. Should crash.
    //TEST_ASSERT_NULL(sbJSON_GetStringValue(number));
    //TEST_ASSERT_NULL(sbJSON_GetStringValue(NULL));
    TEST_ASSERT_NULL(sbJSON_TryGetStringValue(number));
    TEST_ASSERT_NULL(sbJSON_TryGetStringValue(NULL));

    sbJSON_Delete(number);
    sbJSON_Delete(string);
}

static void sbjson_get_number_value_should_get_a_number(void) {
    sbJSON *string = sbJSON_CreateString("test");
    sbJSON *number = sbJSON_CreateIntegerNumber(1);

    TEST_ASSERT_EQUAL_DOUBLE(sbJSON_GetNumberValue(number), (double) number->u.valueint);
    //TODO: Precondition violation. This should crash.
    //TEST_ASSERT_DOUBLE_IS_NAN(sbJSON_GetNumberValue(string));
    TEST_ASSERT_DOUBLE_IS_NAN(sbJSON_GetNumberValue(NULL));

    sbJSON_Delete(number);
    sbJSON_Delete(string);
}

static void
sbjson_create_string_reference_should_create_a_string_reference(void) {
    const char *string = "I am a string!";

    sbJSON *string_reference = sbJSON_CreateStringReference(string);
    TEST_ASSERT_TRUE(string_reference->u.valuestring == string);
    TEST_ASSERT_TRUE(string_reference->type == sbJSON_String);
    TEST_ASSERT_TRUE(string_reference->is_reference);
    TEST_ASSERT_FALSE(string_reference->string_is_const);

    sbJSON_Delete(string_reference);
}

static void
sbjson_create_object_reference_should_create_an_object_reference(void) {
    sbJSON *number_reference = NULL;
    sbJSON *number_object = sbJSON_CreateObject();
    sbJSON *number = sbJSON_CreateIntegerNumber(42);
    const char key[] = "number";

    TEST_ASSERT_TRUE(sbJSON_IsNumber(number));
    TEST_ASSERT_TRUE(sbJSON_IsObject(number_object));
    sbJSON_AddItemToObjectCS(number_object, key, number);

    number_reference = sbJSON_CreateObjectReference(number);
    TEST_ASSERT_TRUE(number_reference->child == number);
    TEST_ASSERT_TRUE(number_reference->type == sbJSON_Object);
    TEST_ASSERT_TRUE(number_reference->is_reference);
    TEST_ASSERT_FALSE(number_reference->string_is_const);

    sbJSON_Delete(number_object);
    sbJSON_Delete(number_reference);
}

static void
sbjson_create_array_reference_should_create_an_array_reference(void) {
    sbJSON *number_reference = NULL;
    sbJSON *number_array = sbJSON_CreateArray();
    sbJSON *number = sbJSON_CreateIntegerNumber(42);

    TEST_ASSERT_TRUE(sbJSON_IsNumber(number));
    TEST_ASSERT_TRUE(sbJSON_IsArray(number_array));
    sbJSON_AddItemToArray(number_array, number);

    number_reference = sbJSON_CreateArrayReference(number);
    TEST_ASSERT_TRUE(number_reference->child == number);
    TEST_ASSERT_TRUE(number_reference->type == sbJSON_Array);
    TEST_ASSERT_TRUE(number_reference->is_reference);
    TEST_ASSERT_FALSE(number_reference->string_is_const);

    sbJSON_Delete(number_array);
    sbJSON_Delete(number_reference);
}

static void sbjson_add_item_to_object_or_array_should_not_add_itself(void) {
    sbJSON *object = sbJSON_CreateObject();
    sbJSON *array = sbJSON_CreateArray();
    bool flag = false;

    flag = sbJSON_AddItemToObject(object, "key", object);
    TEST_ASSERT_FALSE_MESSAGE(flag, "add an object to itself should fail");

    flag = sbJSON_AddItemToArray(array, array);
    TEST_ASSERT_FALSE_MESSAGE(flag, "add an array to itself should fail");

    sbJSON_Delete(object);
    sbJSON_Delete(array);
}

static void
sbjson_add_item_to_object_should_not_use_after_free_when_string_is_aliased(
    void) {
    sbJSON *object = sbJSON_CreateObject();
    sbJSON *number = sbJSON_CreateIntegerNumber(42);
    char *name =
        (char *)sbJSON_strdup((const unsigned char *)"number", &global_hooks);

    TEST_ASSERT_NOT_NULL(object);
    TEST_ASSERT_NOT_NULL(number);
    TEST_ASSERT_NOT_NULL(name);

    number->string = name;

    /* The following should not have a use after free
     * that would show up in valgrind or with AddressSanitizer */
    sbJSON_AddItemToObject(object, number->string, number);

    sbJSON_Delete(object);
}

static void
sbjson_delete_item_from_array_should_not_broken_list_structure(void) {
    const char expected_json1[] = "{\"rd\":[{\"a\":\"123\"}]}";
    const char expected_json2[] = "{\"rd\":[{\"a\":\"123\"},{\"b\":\"456\"}]}";
    const char expected_json3[] = "{\"rd\":[{\"b\":\"456\"}]}";
    char *str1 = NULL;
    char *str2 = NULL;
    char *str3 = NULL;

    sbJSON *root = sbJSON_Parse("{}");

    sbJSON *array = sbJSON_AddArrayToObject(root, "rd");
    sbJSON *item1 = sbJSON_Parse("{\"a\":\"123\"}");
    sbJSON *item2 = sbJSON_Parse("{\"b\":\"456\"}");

    sbJSON_AddItemToArray(array, item1);
    str1 = sbJSON_PrintUnformatted(root);
    TEST_ASSERT_EQUAL_STRING(expected_json1, str1);
    free(str1);

    sbJSON_AddItemToArray(array, item2);
    str2 = sbJSON_PrintUnformatted(root);
    TEST_ASSERT_EQUAL_STRING(expected_json2, str2);
    free(str2);

    /* this should not broken list structure */
    sbJSON_DeleteItemFromArray(array, 0);
    str3 = sbJSON_PrintUnformatted(root);
    TEST_ASSERT_EQUAL_STRING(expected_json3, str3);
    free(str3);

    sbJSON_Delete(root);
}

static void sbjson_set_valuestring_to_object_should_not_leak_memory(void) {
    sbJSON *root = sbJSON_Parse("{}");
    const char *stringvalue = "valuestring could be changed safely";
    const char *reference_valuestring =
        "reference item should be freed by yourself";
    const char *short_valuestring = "shorter valuestring";
    const char *long_valuestring = "new valuestring which is much longer than "
                                   "previous should be changed safely";
    sbJSON *item1 = sbJSON_CreateString(stringvalue);
    sbJSON *item2 = sbJSON_CreateStringReference(reference_valuestring);
    char *ptr1 = NULL;
    char *return_value = NULL;

    sbJSON_AddItemToObject(root, "one", item1);
    sbJSON_AddItemToObject(root, "two", item2);

    ptr1 = item1->u.valuestring;
    return_value = sbJSON_SetValuestring(sbJSON_GetObjectItem(root, "one"),
                                        short_valuestring);
    TEST_ASSERT_NOT_NULL(return_value);
    TEST_ASSERT_EQUAL_PTR_MESSAGE(
        ptr1, return_value,
        "new valuestring shorter than old should not reallocate memory");
    TEST_ASSERT_EQUAL_STRING(short_valuestring,
                             sbJSON_GetObjectItem(root, "one")->u.valuestring);

    /* we needn't to free the original valuestring manually */
    ptr1 = item1->u.valuestring;
    return_value = sbJSON_SetValuestring(sbJSON_GetObjectItem(root, "one"),
                                        long_valuestring);
    TEST_ASSERT_NOT_NULL(return_value);
    TEST_ASSERT_NOT_EQUAL_MESSAGE(
        ptr1, return_value,
        "new valuestring longer than old should reallocate memory");
    TEST_ASSERT_EQUAL_STRING(long_valuestring,
                             sbJSON_GetObjectItem(root, "one")->u.valuestring);

    //TODO: precondition violation; should crash
    /*return_value = sbJSON_SetValuestring(sbJSON_GetObjectItem(root, "two"),*/
                                        /*long_valuestring);*/
    /*TEST_ASSERT_NULL_MESSAGE(*/
        /*return_value, "valuestring of reference object should not be changed");*/
    /*TEST_ASSERT_EQUAL_STRING(reference_valuestring,*/
                             /*sbJSON_GetObjectItem(root, "two")->u.valuestring);*/

    sbJSON_Delete(root);
}

static void sbjson_set_bool_value_must_not_break_objects(void) {
    sbJSON *bobj, *sobj, *oobj, *refobj = NULL;

    TEST_ASSERT_TRUE((sbJSON_SetBoolValue(refobj, 1) == sbJSON_Invalid));

    bobj = sbJSON_CreateFalse();
    TEST_ASSERT_TRUE(sbJSON_IsFalse(bobj));
    TEST_ASSERT_TRUE((sbJSON_SetBoolValue(bobj, 1) == sbJSON_True));
    TEST_ASSERT_TRUE(sbJSON_IsTrue(bobj));
    sbJSON_SetBoolValue(bobj, 1);
    TEST_ASSERT_TRUE(sbJSON_IsTrue(bobj));
    TEST_ASSERT_TRUE((sbJSON_SetBoolValue(bobj, 0) == sbJSON_False));
    TEST_ASSERT_TRUE(sbJSON_IsFalse(bobj));
    sbJSON_SetBoolValue(bobj, 0);
    TEST_ASSERT_TRUE(sbJSON_IsFalse(bobj));

    sobj = sbJSON_CreateString("test");
    TEST_ASSERT_TRUE(sbJSON_IsString(sobj));
    sbJSON_SetBoolValue(sobj, 1);
    TEST_ASSERT_TRUE(sbJSON_IsString(sobj));
    sbJSON_SetBoolValue(sobj, 0);
    TEST_ASSERT_TRUE(sbJSON_IsString(sobj));

    oobj = sbJSON_CreateObject();
    TEST_ASSERT_TRUE(sbJSON_IsObject(oobj));
    sbJSON_SetBoolValue(oobj, 1);
    TEST_ASSERT_TRUE(sbJSON_IsObject(oobj));
    sbJSON_SetBoolValue(oobj, 0);
    TEST_ASSERT_TRUE(sbJSON_IsObject(oobj));

    refobj = sbJSON_CreateStringReference("conststring");
    TEST_ASSERT_TRUE(sbJSON_IsString(refobj));
    TEST_ASSERT_TRUE(refobj->is_reference);
    sbJSON_SetBoolValue(refobj, 1);
    TEST_ASSERT_TRUE(sbJSON_IsString(refobj));
    TEST_ASSERT_TRUE(refobj->is_reference);
    sbJSON_SetBoolValue(refobj, 0);
    TEST_ASSERT_TRUE(sbJSON_IsString(refobj));
    TEST_ASSERT_TRUE(refobj->is_reference);
    sbJSON_Delete(refobj);

    refobj = sbJSON_CreateObjectReference(oobj);
    TEST_ASSERT_TRUE(sbJSON_IsObject(refobj));
    TEST_ASSERT_TRUE(refobj->is_reference);
    sbJSON_SetBoolValue(refobj, 1);
    TEST_ASSERT_TRUE(sbJSON_IsObject(refobj));
    TEST_ASSERT_TRUE(refobj->is_reference);
    sbJSON_SetBoolValue(refobj, 0);
    TEST_ASSERT_TRUE(sbJSON_IsObject(refobj));
    TEST_ASSERT_TRUE(refobj->is_reference);
    sbJSON_Delete(refobj);

    sbJSON_Delete(oobj);
    sbJSON_Delete(bobj);
    sbJSON_Delete(sobj);
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
