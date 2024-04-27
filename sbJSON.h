/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

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

/* sbJSON Types: */
enum sbJSON_Kind {
    sbJSON_Invalid,
    sbJSON_False,
    sbJSON_True,
    sbJSON_NULL,
    sbJSON_Number,
    sbJSON_String,
    sbJSON_Array,
    sbJSON_Object,
    sbJSON_Raw,
};

/* The sbJSON structure: */
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

    union U {
        /* The item's string, if type==sbJSON_String  and type == sbJSON_Raw */
        char *valuestring;
        /* The item's number, if type==sbJSON_Number */
        int64_t valueint;
        double valuedouble;
    } u;

    bool is_number_double;

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

/* Memory Management: the caller is always responsible to free the results from
 * all variants of sbJSON_Parse (with sbJSON_Delete) and sbJSON_Print (with
 * stdlib free, sbJSON_Hooks.free_fn, or sbJSON_free as appropriate). The
 * exception is sbJSON_PrintPreallocated, where the caller has full
 * responsibility of the buffer. */
/* Supply a block of JSON, and this returns a sbJSON object you can interrogate.
 */
sbJSON *sbJSON_Parse(const char *value);
sbJSON *sbJSON_ParseWithLength(const char *value, size_t buffer_length);
/* ParseWithOpts allows you to require (and check) that the JSON is null
 * terminated, and to retrieve the pointer to the final byte parsed. */
/* If you supply a ptr in return_parse_end and parsing fails, then
 * return_parse_end will contain a pointer to the error so will match
 * sbJSON_GetErrorPtr(). */
sbJSON *sbJSON_ParseWithOpts(const char *value, const char **return_parse_end,
                             bool require_null_terminated);
sbJSON *sbJSON_ParseWithLengthOpts(const char *value, size_t buffer_length,
                                   const char **return_parse_end,
                                   bool require_null_terminated);

/* Render a sbJSON entity to text for transfer/storage. */
char *sbJSON_Print(const sbJSON *item);
/* Render a sbJSON entity to text for transfer/storage without any formatting.
 */
char *sbJSON_PrintUnformatted(const sbJSON *item);
/* Render a sbJSON entity to text using a buffered strategy. prebuffer is a
 * guess at the final size. guessing well reduces reallocation. fmt=0 gives
 * unformatted, =1 gives formatted */
char *sbJSON_PrintBuffered(const sbJSON *item, int prebuffer, bool fmt);
/* Render a sbJSON entity to text using a buffer already allocated in memory
 * with given length. Returns 1 on success and 0 on failure. */
/* NOTE: sbJSON is not always 100% accurate in estimating how much memory it
 * will use, so to be safe allocate 5 bytes more than you actually need */
bool sbJSON_PrintPreallocated(sbJSON *item, char *buffer, const int length,
                              const bool format);
/* Delete a sbJSON entity and all subentities. */
void sbJSON_Delete(sbJSON *item);

/* Returns the number of items in an array (or object). */
int sbJSON_GetArraySize(const sbJSON *array);
/* Retrieve item number "index" from array "array". Returns NULL if
 * unsuccessful. */
sbJSON *sbJSON_GetArrayItem(const sbJSON *array, int index);
/* Get item "string" from object. Case insensitive. */
sbJSON *sbJSON_GetObjectItem(const sbJSON *const object,
                             const char *const string);
sbJSON *sbJSON_GetObjectItemCaseSensitive(const sbJSON *const object,
                                          const char *const string);
bool sbJSON_HasObjectItem(const sbJSON *object, const char *string);
/* For analysing failed parses. This returns a pointer to the parse error.
 * You'll probably need to look a few chars back to make sense of it. Defined
 * when sbJSON_Parse() returns 0. 0 when sbJSON_Parse() succeeds. */
const char *sbJSON_GetErrorPtr(void);

/* Check item type and return its value */
char *sbJSON_GetStringValue(const sbJSON *const item);
double sbJSON_GetNumberValue(const sbJSON *const item);

/* These functions check the type of an item */
bool sbJSON_IsInvalid(const sbJSON *const item);
bool sbJSON_IsFalse(const sbJSON *const item);
bool sbJSON_IsTrue(const sbJSON *const item);
bool sbJSON_IsBool(const sbJSON *const item);
bool sbJSON_IsNull(const sbJSON *const item);
bool sbJSON_IsNumber(const sbJSON *const item);
bool sbJSON_IsString(const sbJSON *const item);
bool sbJSON_IsArray(const sbJSON *const item);
bool sbJSON_IsObject(const sbJSON *const item);
bool sbJSON_IsRaw(const sbJSON *const item);

/* These calls create a sbJSON item of the appropriate type. */
sbJSON *sbJSON_CreateNull(void);
sbJSON *sbJSON_CreateTrue(void);
sbJSON *sbJSON_CreateFalse(void);
sbJSON *sbJSON_CreateBool(bool boolean);
sbJSON *sbJSON_CreateDoubleNumber(double num);
sbJSON *sbJSON_CreateIntegerNumber(int64_t num);
sbJSON *sbJSON_CreateString(const char *string);
/* raw json */
sbJSON *sbJSON_CreateRaw(const char *raw);
sbJSON *sbJSON_CreateArray(void);
sbJSON *sbJSON_CreateObject(void);

/* Create a string where valuestring references a string so
 * it will not be freed by sbJSON_Delete */
sbJSON *sbJSON_CreateStringReference(const char *string);
/* Create an object/array that only references it's elements so
 * they will not be freed by sbJSON_Delete */
sbJSON *sbJSON_CreateObjectReference(const sbJSON *child);
sbJSON *sbJSON_CreateArrayReference(const sbJSON *child);

/* These utilities create an Array of count items.
 * The parameter count cannot be greater than the number of elements in the
 * number array, otherwise array access will be out of bounds.*/
sbJSON *sbJSON_CreateIntArray(const int *numbers, int count);
sbJSON *sbJSON_CreateFloatArray(const float *numbers, int count);
sbJSON *sbJSON_CreateDoubleArray(const double *numbers, int count);
sbJSON *sbJSON_CreateStringArray(const char *const *strings, int count);

/* Append item to the specified array/object. */
bool sbJSON_AddItemToArray(sbJSON *array, sbJSON *item);
bool sbJSON_AddItemToObject(sbJSON *object, const char *string, sbJSON *item);
/* Use this when string is definitely const (i.e. a literal, or as good as), and
 * will definitely survive the sbJSON object. WARNING: When this function was
 * used, make sure to always check that item->string_is_const is false
 * before writing to `item->string` */
bool sbJSON_AddItemToObjectCS(sbJSON *object, const char *string, sbJSON *item);
/* Append reference to item to the specified array/object. Use this when you
 * want to add an existing sbJSON to a new sbJSON, but don't want to corrupt
 * your existing sbJSON. */
bool sbJSON_AddItemReferenceToArray(sbJSON *array, sbJSON *item);
bool sbJSON_AddItemReferenceToObject(sbJSON *object, const char *string,
                                     sbJSON *item);

/* Remove/Detach items from Arrays/Objects. */
sbJSON *sbJSON_DetachItemViaPointer(sbJSON *parent, sbJSON *const item);
sbJSON *sbJSON_DetachItemFromArray(sbJSON *array, int which);
void sbJSON_DeleteItemFromArray(sbJSON *array, int which);
sbJSON *sbJSON_DetachItemFromObject(sbJSON *object, const char *string);
sbJSON *sbJSON_DetachItemFromObjectCaseSensitive(sbJSON *object,
                                                 const char *string);
void sbJSON_DeleteItemFromObject(sbJSON *object, const char *string);
void sbJSON_DeleteItemFromObjectCaseSensitive(sbJSON *object,
                                              const char *string);

/* Update array items. */
bool sbJSON_InsertItemInArray(
    sbJSON *array, int which,
    sbJSON *newitem); /* Shifts pre-existing items to the right. */
bool sbJSON_ReplaceItemViaPointer(sbJSON *const parent, sbJSON *const item,
                                  sbJSON *replacement);
bool sbJSON_ReplaceItemInArray(sbJSON *array, int which, sbJSON *newitem);
bool sbJSON_ReplaceItemInObject(sbJSON *object, const char *string,
                                sbJSON *newitem);
bool sbJSON_ReplaceItemInObjectCaseSensitive(sbJSON *object, const char *string,
                                             sbJSON *newitem);

/* Duplicate a sbJSON item */
sbJSON *sbJSON_Duplicate(const sbJSON *item, bool recurse);
/* Duplicate will create a new, identical sbJSON item to the one you pass, in
 * new memory that will need to be released. With recurse!=0, it will duplicate
 * any children connected to the item. The item->next and ->prev pointers are
 * always zero on return from Duplicate. */
/* Recursively compare two sbJSON items for equality. If either a or b is NULL
 * or invalid, they will be considered unequal. case_sensitive determines if
 * object keys are treated case sensitive (1) or case insensitive (0) */
bool sbJSON_Compare(const sbJSON *const a, const sbJSON *const b,
                    const bool case_sensitive);

/* Minify a strings, remove blank characters(such as ' ', '\t', '\r', '\n') from
 * strings. The input pointer json cannot point to a read-only address area,
 * such as a string constant, but should point to a readable and writable
 * address area. */
void sbJSON_Minify(char *json);

/* Helper functions for creating and adding items to an object at the same time.
 * They return the added item or NULL on failure. */
sbJSON *sbJSON_AddNullToObject(sbJSON *const object, const char *const name);
sbJSON *sbJSON_AddTrueToObject(sbJSON *const object, const char *const name);
sbJSON *sbJSON_AddFalseToObject(sbJSON *const object, const char *const name);
sbJSON *sbJSON_AddBoolToObject(sbJSON *const object, const char *const name,
                               const bool boolean);
sbJSON *sbJSON_AddDoubleNumberToObject(sbJSON *const object,
                                       const char *const name,
                                       const double number);
sbJSON *sbJSON_AddIntegerNumberToObject(sbJSON *const object,
                                        const char *const name, int64_t number);
sbJSON *sbJSON_AddStringToObject(sbJSON *const object, const char *const name,
                                 const char *const string);
sbJSON *sbJSON_AddRawToObject(sbJSON *const object, const char *const name,
                              const char *const raw);
sbJSON *sbJSON_AddObjectToObject(sbJSON *const object, const char *const name);
sbJSON *sbJSON_AddArrayToObject(sbJSON *const object, const char *const name);

double sbJSON_SetDoubleNumberValue(sbJSON *object, double number);
int64_t sbJSON_SetIntegerNumberValue(sbJSON *object, int64_t number);

/* Change the valuestring of a sbJSON_String object, only takes effect when type
 * of object is sbJSON_String */
char *sbJSON_SetValuestring(sbJSON *object, const char *valuestring);

/* If the object is not a boolean type this does nothing and returns
 * sbJSON_Invalid else it returns the new type*/
enum sbJSON_Kind sbJSON_SetBoolValue(sbJSON *object, bool boolValue);

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
