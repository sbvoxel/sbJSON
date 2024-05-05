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

#ifndef sbJSON__h
#define sbJSON__h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum sbJSON_Kind {
    sbJSON_Invalid, // TODO: Document whether parser can return this.
    sbJSON_Bool,
    sbJSON_Null,
    sbJSON_Number,
    sbJSON_String,
    sbJSON_Array,
    sbJSON_Object,
    sbJSON_Raw,
};

typedef struct sbJSON {
    /* next/prev allow you to walk array/object chains. Alternatively, use
     * GetArraySize/GetArrayItem/GetObjectItem */
    struct sbJSON *next;
    struct sbJSON *prev;
    /* An array or object item will have a child pointer pointing to a chain of
     * the items in the array/object. */
    struct sbJSON *child;

    /* The type of the item, as above. */
    uint8_t type;
    bool is_reference;
    bool string_is_const;
    bool is_number_double;

    union U {
        char *valuestring;
        int64_t valueint;
        double valuedouble;
        bool valuebool;
    } u;

    /* The item's name string, if this item is the child of, or is in the list
     * of subitems of an object. */
    char *string;
} sbJSON;

typedef struct sbJSON_Hooks {
    void *(*malloc_fn)(size_t sz);
    void (*free_fn)(void *ptr);
} sbJSON_Hooks;

/* Limits how deeply nested arrays/objects can be before sbJSON rejects to parse
 * them. This is to prevent stack overflows. */
#ifndef SBJSON_NESTING_LIMIT
#define SBJSON_NESTING_LIMIT 1000
#endif

/* Supply malloc, realloc and free functions to sbJSON */
void sbJSON_InitHooks(sbJSON_Hooks *hooks);

sbJSON *sbj_parse(char const *value);
sbJSON *sbj_parse_with_length(char const *value, size_t buffer_length);
sbJSON *sbj_parse_with_opts(char const *value, char const **return_parse_end,
                             bool require_null_terminated);
sbJSON *sbj_parse_with_length_opts(char const *value, size_t buffer_length,
                                   char const **return_parse_end,
                                   bool require_null_terminated);

char *sbj_print(sbJSON const *item);
char *sbj_print_unformatted(sbJSON const *item);
char *sbj_print_buffered(sbJSON const *item, int prebuffer, bool fmt);
bool sbj_print_preallocated(sbJSON *item, char *buffer, int const length,
                              bool const format);
void sbj_delete(sbJSON *item);

int sbj_get_array_size(sbJSON const *array);
sbJSON *sbj_get_array_item(sbJSON const *array, int index);
sbJSON *sbj_get_object_item(sbJSON const *const object,
                             char const *const string);
bool sbj_has_object_item(sbJSON const *object, char const *string);
char const *sbJSON_GetErrorPtr(void);

/* Get values of items of known type */
char *sbj_get_string_value(sbJSON const *const item);
double sbj_get_number_value(sbJSON const *const item);
bool sbj_get_bool_value(sbJSON const *const item);

/* Get values of items of unknown type */
char *sbj_try_get_string_value(sbJSON const *const item);
double sbj_try_get_number_value(sbJSON const *const item);
bool sbj_try_get_bool_value(sbJSON const *const item, bool default_bool);

bool sbj_is_invalid(sbJSON const *const item);
bool sbj_is_bool(sbJSON const *const item);
bool sbj_is_null(sbJSON const *const item);
bool sbj_is_number(sbJSON const *const item);
bool sbj_is_string(sbJSON const *const item);
bool sbj_is_array(sbJSON const *const item);
bool sbj_is_object(sbJSON const *const item);
bool sbj_is_raw(sbJSON const *const item);

sbJSON *sbj_create_null(void);
sbJSON *sbj_create_bool(bool boolean);
sbJSON *sbj_create_double_number(double num);
sbJSON *sbj_create_integer_number(int64_t num);
sbJSON *sbJSON_CreateString(char const *string);
sbJSON *sbj_create_raw(char const *raw);
sbJSON *sbj_create_array(void);
sbJSON *sbj_create_object(void);

/* Create a string where valuestring references a string so
 * it will not be freed by sbj_delete */
sbJSON *sbj_create_string_reference(char const *string);
/* Create an object/array that only references its elements so
 * they will not be freed by sbj_delete */
sbJSON *sbj_create_object_reference(sbJSON const *child);
sbJSON *sbj_create_array_reference(sbJSON const *child);

/* These utilities create an Array of count items. */
sbJSON *sbj_create_int_array(int const *numbers, int count);
sbJSON *sbj_create_float_array(float const *numbers, int count);
sbJSON *sbj_create_double_array(double const *numbers, int count);
sbJSON *sbj_create_string_array(char const *const *strings, int count);

/* Append item to the specified array/object. */
bool sbj_add_item_to_array(sbJSON *array, sbJSON *item);
bool sbj_add_item_to_object(sbJSON *object, char const *string, sbJSON *item);
/* Use this when string is definitely const (i.e. a literal, or as good as), and
 * will definitely survive the sbJSON object. WARNING: When this function was
 * used, make sure to always check that item->string_is_const is false
 * before writing to `item->string` */
bool sbj_add_item_to_objectCS(sbJSON *object, char const *string, sbJSON *item);
/* Append reference to item to the specified array/object. Use this when you
 * want to add an existing sbJSON to a new sbJSON, but don't want to corrupt
 * your existing sbJSON. */
bool sbj_add_item_reference_to_array(sbJSON *array, sbJSON *item);
bool sbj_add_item_reference_to_object(sbJSON *object, char const *string,
                                     sbJSON *item);

sbJSON *sbj_detach_item_via_pointer(sbJSON *parent, sbJSON *const item);
sbJSON *sbj_detach_item_from_array(sbJSON *array, int which);
void sbj_delete_item_from_array(sbJSON *array, int which);
sbJSON *sbj_detach_item_from_object(sbJSON *object, char const *string);
void sbj_delete_item_from_object(sbJSON *object, char const *string);

bool sbj_insert_item_in_array(
    sbJSON *array, int which,
    sbJSON *newitem); /* Shifts pre-existing items to the right. */
bool sbj_replace_item_via_pointer(sbJSON *const parent, sbJSON *const item,
                                  sbJSON *replacement);
bool sbj_replace_item_in_array(sbJSON *array, int which, sbJSON *newitem);
bool sbj_replace_item_in_object(sbJSON *object, char const *string,
                                sbJSON *newitem);

/* Duplicate a sbJSON item */
sbJSON *sbj_duplicate(sbJSON const *item, bool recurse);

bool sbj_compare(sbJSON const *const a, sbJSON const *const b);

void sbj_minify(char *json);

sbJSON *sbj_add_null_to_object(sbJSON *const object, char const *const name);
sbJSON *sbj_add_true_to_object(sbJSON *const object, char const *const name);
sbJSON *sbj_add_false_to_object(sbJSON *const object, char const *const name);
sbJSON *sbj_add_bool_to_object(sbJSON *const object, char const *const name,
                               bool const boolean);
sbJSON *sbj_add_double_number_to_object(sbJSON *const object,
                                       char const *const name,
                                       double const number);
sbJSON *sbj_add_integer_number_to_object(sbJSON *const object,
                                        char const *const name, int64_t number);
sbJSON *sbj_add_string_to_object(sbJSON *const object, char const *const name,
                                 char const *const string);
sbJSON *sbj_add_raw_to_object(sbJSON *const object, char const *const name,
                              char const *const raw);
sbJSON *sbj_add_object_to_object(sbJSON *const object, char const *const name);
sbJSON *sbj_add_array_to_object(sbJSON *const object, char const *const name);

void sbj_set_double_number_value(sbJSON *object, double number);
void sbj_set_integer_number_value(sbJSON *object, int64_t number);

char *sbj_set_valuestring(sbJSON *object, char const *valuestring);

bool sbj_set_bool_value(sbJSON *object, bool boolValue);

/* Macro for iterating over an array or object */
#define sbJSON_ArrayForEach(element, array)                                    \
    for (element = (array != NULL) ? (array)->child : NULL; element != NULL;   \
         element = element->next)

/* malloc/free objects using the malloc/free functions that have been set with
 * sbJSON_InitHooks */
void *sbJSON_malloc(size_t size);
void sbJSON_free(void *object);

#ifdef __cplusplus
}
#endif

#endif
