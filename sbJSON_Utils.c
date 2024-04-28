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

#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sbJSON_Utils.h"

static unsigned char *sbJSONUtils_strdup(const unsigned char *const string) {
    size_t length = 0;
    unsigned char *copy = NULL;

    length = strlen((const char *)string) + sizeof("");
    copy = (unsigned char *)sbJSON_malloc(length);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, string, length);

    return copy;
}

/* string comparison which doesn't consider NULL pointers equal */
static int compare_strings(const unsigned char *string1,
                           const unsigned char *string2) {
    if ((string1 == NULL) || (string2 == NULL)) {
        return 1;
    }

    if (string1 == string2) {
        return 0;
    }

    return strcmp((const char *)string1, (const char *)string2);
}

/* securely comparison of floating-point variables */
static bool compare_double(double a, double b) {
    double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
    return (fabs(a - b) <= maxVal * DBL_EPSILON);
}

/* Compare the next path element of two JSON pointers, two NULL pointers are
 * considered unequal: */
static bool compare_pointers(const unsigned char *name,
                             const unsigned char *pointer) {
    if ((name == NULL) || (pointer == NULL)) {
        return false;
    }

    for (; (*name != '\0') && (*pointer != '\0') && (*pointer != '/');
         (void)name++, pointer++) /* compare until next '/' */
    {
        if (*pointer == '~') {
            /* check for escaped '~' (~0) and '/' (~1) */
            if (((pointer[1] != '0') || (*name != '~')) &&
                ((pointer[1] != '1') || (*name != '/'))) {
                /* invalid escape sequence or wrong character in *name */
                return false;
            } else {
                pointer++;
            }
        } else if (*name != *pointer) {
            return false;
        }
    }
    if (((*pointer != 0) && (*pointer != '/')) != (*name != 0)) {
        /* one string has ended, the other not */
        return false;
        ;
    }

    return true;
}

/* calculate the length of a string if encoded as JSON pointer with ~0 and ~1
 * escape sequences */
static size_t pointer_encoded_length(const unsigned char *string) {
    size_t length;
    for (length = 0; *string != '\0'; (void)string++, length++) {
        /* character needs to be escaped? */
        if ((*string == '~') || (*string == '/')) {
            length++;
        }
    }

    return length;
}

/* copy a string while escaping '~' and '/' with ~0 and ~1 JSON pointer escape
 * codes */
static void encode_string_as_pointer(unsigned char *destination,
                                     const unsigned char *source) {
    for (; source[0] != '\0'; (void)source++, destination++) {
        if (source[0] == '/') {
            destination[0] = '~';
            destination[1] = '1';
            destination++;
        } else if (source[0] == '~') {
            destination[0] = '~';
            destination[1] = '0';
            destination++;
        } else {
            destination[0] = source[0];
        }
    }

    destination[0] = '\0';
}

char *sbJSONUtils_FindPointerFromObjectTo(const sbJSON *const object,
                                          const sbJSON *const target) {
    size_t child_index = 0;
    sbJSON *current_child = 0;

    if ((object == NULL) || (target == NULL)) {
        return NULL;
    }

    if (object == target) {
        /* found */
        return (char *)sbJSONUtils_strdup((const unsigned char *)"");
    }

    /* recursively search all children of the object or array */
    for (current_child = object->child; current_child != NULL;
         (void)(current_child = current_child->next), child_index++) {
        unsigned char *target_pointer =
            (unsigned char *)sbJSONUtils_FindPointerFromObjectTo(current_child,
                                                                 target);
        /* found the target? */
        if (target_pointer != NULL) {
            if (sbJSON_IsArray(object)) {
                /* reserve enough memory for a 64 bit integer + '/' and '\0' */
                unsigned char *full_pointer = (unsigned char *)sbJSON_malloc(
                    strlen((char *)target_pointer) + 20 + sizeof("/"));
                /* check if conversion to unsigned long is valid
                 * This should be eliminated at compile time by dead code
                 * elimination if size_t is an alias of unsigned long, or if it
                 * is bigger */
                if (child_index > ULONG_MAX) {
                    sbJSON_free(target_pointer);
                    sbJSON_free(full_pointer);
                    return NULL;
                }
                sprintf((char *)full_pointer, "/%lu%s",
                        (unsigned long)child_index,
                        target_pointer); /* /<array_index><path> */
                sbJSON_free(target_pointer);

                return (char *)full_pointer;
            }

            if (sbJSON_IsObject(object)) {
                unsigned char *full_pointer = (unsigned char *)sbJSON_malloc(
                    strlen((char *)target_pointer) +
                    pointer_encoded_length(
                        (unsigned char *)current_child->string) +
                    2);
                full_pointer[0] = '/';
                encode_string_as_pointer(
                    full_pointer + 1, (unsigned char *)current_child->string);
                strcat((char *)full_pointer, (char *)target_pointer);
                sbJSON_free(target_pointer);

                return (char *)full_pointer;
            }

            /* reached leaf of the tree, found nothing */
            sbJSON_free(target_pointer);
            return NULL;
        }
    }

    /* not found */
    return NULL;
}

/* non broken version of sbJSON_GetArrayItem */
static sbJSON *get_array_item(const sbJSON *array, size_t item) {
    sbJSON *child = array ? array->child : NULL;
    while ((child != NULL) && (item > 0)) {
        item--;
        child = child->next;
    }

    return child;
}

static bool decode_array_index_from_pointer(const unsigned char *const pointer,
                                            size_t *const index) {
    size_t parsed_index = 0;
    size_t position = 0;

    if ((pointer[0] == '0') && ((pointer[1] != '\0') && (pointer[1] != '/'))) {
        /* leading zeroes are not permitted */
        return 0;
    }

    for (position = 0; (pointer[position] >= '0') && (pointer[0] <= '9');
         position++) {
        parsed_index = (10 * parsed_index) + (size_t)(pointer[position] - '0');
    }

    if ((pointer[position] != '\0') && (pointer[position] != '/')) {
        return 0;
    }

    *index = parsed_index;

    return 1;
}

static sbJSON *get_item_from_pointer(sbJSON *const object, const char *pointer) {
    sbJSON *current_element = object;

    if (pointer == NULL) {
        return NULL;
    }

    /* follow path of the pointer */
    while ((pointer[0] == '/') && (current_element != NULL)) {
        pointer++;
        if (sbJSON_IsArray(current_element)) {
            size_t index = 0;
            if (!decode_array_index_from_pointer((const unsigned char *)pointer,
                                                 &index)) {
                return NULL;
            }

            current_element = get_array_item(current_element, index);
        } else if (sbJSON_IsObject(current_element)) {
            current_element = current_element->child;
            /* GetObjectItem. */
            while ((current_element != NULL) &&
                   !compare_pointers((unsigned char *)current_element->string,
                                     (const unsigned char *)pointer)) {
                current_element = current_element->next;
            }
        } else {
            return NULL;
        }

        /* skip to the next path token or end of string */
        while ((pointer[0] != '\0') && (pointer[0] != '/')) {
            pointer++;
        }
    }

    return current_element;
}

sbJSON *sbJSONUtils_GetPointer(sbJSON *const object, const char *pointer) {
    return get_item_from_pointer(object, pointer);
}

/* JSON Patch implementation. */
static void decode_pointer_inplace(unsigned char *string) {
    unsigned char *decoded_string = string;

    if (string == NULL) {
        return;
    }

    for (; *string; (void)decoded_string++, string++) {
        if (string[0] == '~') {
            if (string[1] == '0') {
                decoded_string[0] = '~';
            } else if (string[1] == '1') {
                decoded_string[1] = '/';
            } else {
                /* invalid escape sequence */
                return;
            }

            string++;
        }
    }

    decoded_string[0] = '\0';
}

/* non-broken sbJSON_DetachItemFromArray */
static sbJSON *detach_item_from_array(sbJSON *array, size_t which) {
    sbJSON *c = array->child;
    while (c && (which > 0)) {
        c = c->next;
        which--;
    }
    if (!c) {
        /* item doesn't exist */
        return NULL;
    }
    if (c != array->child) {
        /* not the first element */
        c->prev->next = c->next;
    }
    if (c->next) {
        c->next->prev = c->prev;
    }
    if (c == array->child) {
        array->child = c->next;
    } else if (c->next == NULL) {
        array->child->prev = c->prev;
    }
    /* make sure the detached item doesn't point anywhere anymore */
    c->prev = c->next = NULL;

    return c;
}

/* detach an item at the given path */
static sbJSON *detach_path(sbJSON *object, const unsigned char *path) {
    unsigned char *parent_pointer = NULL;
    unsigned char *child_pointer = NULL;
    sbJSON *parent = NULL;
    sbJSON *detached_item = NULL;

    /* copy path and split it in parent and child */
    parent_pointer = sbJSONUtils_strdup(path);
    if (parent_pointer == NULL) {
        goto cleanup;
    }

    child_pointer =
        (unsigned char *)strrchr((char *)parent_pointer, '/'); /* last '/' */
    if (child_pointer == NULL) {
        goto cleanup;
    }
    /* split strings */
    child_pointer[0] = '\0';
    child_pointer++;

    parent =
        get_item_from_pointer(object, (char *)parent_pointer);
    decode_pointer_inplace(child_pointer);

    if (sbJSON_IsArray(parent)) {
        size_t index = 0;
        if (!decode_array_index_from_pointer(child_pointer, &index)) {
            goto cleanup;
        }
        detached_item = detach_item_from_array(parent, index);
    } else if (sbJSON_IsObject(parent)) {
        detached_item =
            sbJSON_DetachItemFromObject(parent, (char *)child_pointer);
    } else {
        /* Couldn't find object to remove child from. */
        goto cleanup;
    }

cleanup:
    if (parent_pointer != NULL) {
        sbJSON_free(parent_pointer);
    }

    return detached_item;
}

/* sort lists using mergesort */
static sbJSON *sort_list(sbJSON *list) {
    sbJSON *first = list;
    sbJSON *second = list;
    sbJSON *current_item = list;
    sbJSON *result = list;
    sbJSON *result_tail = NULL;

    if ((list == NULL) || (list->next == NULL)) {
        /* One entry is sorted already. */
        return result;
    }

    while ((current_item != NULL) && (current_item->next != NULL) &&
           (compare_strings((unsigned char *)current_item->string,
                            (unsigned char *)current_item->next->string) < 0)) {
        /* Test for list sorted. */
        current_item = current_item->next;
    }
    if ((current_item == NULL) || (current_item->next == NULL)) {
        /* Leave sorted lists unmodified. */
        return result;
    }

    /* reset pointer to the beginning */
    current_item = list;
    while (current_item != NULL) {
        /* Walk two pointers to find the middle. */
        second = second->next;
        current_item = current_item->next;
        /* advances current_item two steps at a time */
        if (current_item != NULL) {
            current_item = current_item->next;
        }
    }
    if ((second != NULL) && (second->prev != NULL)) {
        /* Split the lists */
        second->prev->next = NULL;
        second->prev = NULL;
    }

    /* Recursively sort the sub-lists. */
    first = sort_list(first);
    second = sort_list(second);
    result = NULL;

    /* Merge the sub-lists */
    while ((first != NULL) && (second != NULL)) {
        sbJSON *smaller = NULL;
        if (compare_strings((unsigned char *)first->string,
                            (unsigned char *)second->string) < 0) {
            smaller = first;
        } else {
            smaller = second;
        }

        if (result == NULL) {
            /* start merged list with the smaller element */
            result_tail = smaller;
            result = smaller;
        } else {
            /* add smaller element to the list */
            result_tail->next = smaller;
            smaller->prev = result_tail;
            result_tail = smaller;
        }

        if (first == smaller) {
            first = first->next;
        } else {
            second = second->next;
        }
    }

    if (first != NULL) {
        /* Append rest of first list. */
        if (result == NULL) {
            return first;
        }
        result_tail->next = first;
        first->prev = result_tail;
    }
    if (second != NULL) {
        /* Append rest of second list */
        if (result == NULL) {
            return second;
        }
        result_tail->next = second;
        second->prev = result_tail;
    }

    return result;
}

static void sort_object(sbJSON *const object) {
    if (object == NULL) {
        return;
    }
    object->child = sort_list(object->child);
}

static bool numbers_match(sbJSON *a, sbJSON *b) {
    if (a->is_number_double != b->is_number_double) {
        return false;
    }

    if (a->is_number_double) {
        return compare_double(a->u.valuedouble, b->u.valuedouble);
    } else {
        return a->u.valueint == b->u.valueint;
    }
}

static bool compare_json(sbJSON *a, sbJSON *b) {
    if ((a == NULL) || (b == NULL) || ((a->type != b->type))) {
        /* mismatched type. */
        return false;
    }
    switch (a->type) {
    case sbJSON_Number:
        return numbers_match(a, b);

    case sbJSON_String:
        /* string mismatch. */
        if (strcmp(a->u.valuestring, b->u.valuestring) != 0) {
            return false;
        } else {
            return true;
        }

    case sbJSON_Array:
        for ((void)(a = a->child), b = b->child; (a != NULL) && (b != NULL);
             (void)(a = a->next), b = b->next) {
            bool identical = compare_json(a, b);
            if (!identical) {
                return false;
            }
        }

        /* array size mismatch? (one of both children is not NULL) */
        if ((a != NULL) || (b != NULL)) {
            return false;
        } else {
            return true;
        }

    case sbJSON_Object:
        sort_object(a);
        sort_object(b);
        for ((void)(a = a->child), b = b->child; (a != NULL) && (b != NULL);
             (void)(a = a->next), b = b->next) {
            bool identical = false;
            /* compare object keys */
            if (compare_strings((unsigned char *)a->string,
                                (unsigned char *)b->string)) {
                /* missing member */
                return false;
            }
            identical = compare_json(a, b);
            if (!identical) {
                return false;
            }
        }

        /* object length mismatch (one of both children is not null) */
        if ((a != NULL) || (b != NULL)) {
            return false;
        } else {
            return true;
        }

    default:
        break;
    }

    /* null, true or false */
    return true;
}

/* non broken version of sbJSON_InsertItemInArray */
static bool insert_item_in_array(sbJSON *array, size_t which, sbJSON *newitem) {
    sbJSON *child = array->child;
    while (child && (which > 0)) {
        child = child->next;
        which--;
    }
    if (which > 0) {
        /* item is after the end of the array */
        return 0;
    }
    if (child == NULL) {
        sbJSON_AddItemToArray(array, newitem);
        return 1;
    }

    /* insert into the linked list */
    newitem->next = child;
    newitem->prev = child->prev;
    child->prev = newitem;

    /* was it at the beginning */
    if (child == array->child) {
        array->child = newitem;
    } else {
        newitem->prev->next = newitem;
    }

    return 1;
}

static sbJSON *get_object_item(const sbJSON *const object, const char *name) {
    return sbJSON_GetObjectItem(object, name);
}

enum patch_operation { INVALID, ADD, REMOVE, REPLACE, MOVE, COPY, TEST };

static enum patch_operation decode_patch_operation(const sbJSON *const patch) {
    sbJSON *operation = get_object_item(patch, "op");
    if (!sbJSON_IsString(operation)) {
        return INVALID;
    }

    if (strcmp(operation->u.valuestring, "add") == 0) {
        return ADD;
    }

    if (strcmp(operation->u.valuestring, "remove") == 0) {
        return REMOVE;
    }

    if (strcmp(operation->u.valuestring, "replace") == 0) {
        return REPLACE;
    }

    if (strcmp(operation->u.valuestring, "move") == 0) {
        return MOVE;
    }

    if (strcmp(operation->u.valuestring, "copy") == 0) {
        return COPY;
    }

    if (strcmp(operation->u.valuestring, "test") == 0) {
        return TEST;
    }

    return INVALID;
}

/* overwrite and existing item with another one and free resources on the way */
static void overwrite_item(sbJSON *const root, const sbJSON replacement) {
    if (root == NULL) {
        return;
    }

    if (root->string != NULL) {
        sbJSON_free(root->string);
    }
    if (root->u.valuestring != NULL) {
        sbJSON_free(root->u.valuestring);
    }
    if (root->child != NULL) {
        sbJSON_Delete(root->child);
    }

    memcpy(root, &replacement, sizeof(sbJSON));
}

static int apply_patch(sbJSON *object, const sbJSON *patch) {
    sbJSON *path = NULL;
    sbJSON *value = NULL;
    sbJSON *parent = NULL;
    enum patch_operation opcode = INVALID;
    unsigned char *parent_pointer = NULL;
    unsigned char *child_pointer = NULL;
    int status = 0;

    path = get_object_item(patch, "path");
    if (!sbJSON_IsString(path)) {
        /* malformed patch. */
        status = 2;
        goto cleanup;
    }

    opcode = decode_patch_operation(patch);
    if (opcode == INVALID) {
        status = 3;
        goto cleanup;
    } else if (opcode == TEST) {
        /* compare value: {...} with the given path */
        status = !compare_json(
            get_item_from_pointer(object, path->u.valuestring),
            get_object_item(patch, "value"));
        goto cleanup;
    }

    /* special case for replacing the root */
    if (path->u.valuestring[0] == '\0') {
        if (opcode == REMOVE) {
            static const sbJSON invalid = {
                NULL, NULL, NULL, sbJSON_Invalid, 0, 0, {0}, false, NULL};

            overwrite_item(object, invalid);

            status = 0;
            goto cleanup;
        }

        if ((opcode == REPLACE) || (opcode == ADD)) {
            value = get_object_item(patch, "value");
            if (value == NULL) {
                /* missing "value" for add/replace. */
                status = 7;
                goto cleanup;
            }

            value = sbJSON_Duplicate(value, 1);
            if (value == NULL) {
                /* out of memory for add/replace. */
                status = 8;
                goto cleanup;
            }

            overwrite_item(object, *value);

            /* delete the duplicated value */
            sbJSON_free(value);
            value = NULL;

            /* the string "value" isn't needed */
            if (object->string != NULL) {
                sbJSON_free(object->string);
                object->string = NULL;
            }

            status = 0;
            goto cleanup;
        }
    }

    if ((opcode == REMOVE) || (opcode == REPLACE)) {
        /* Get rid of old. */
        sbJSON *old_item = detach_path(
            object, (unsigned char *)path->u.valuestring);
        if (old_item == NULL) {
            status = 13;
            goto cleanup;
        }
        sbJSON_Delete(old_item);
        if (opcode == REMOVE) {
            /* For Remove, this job is done. */
            status = 0;
            goto cleanup;
        }
    }

    /* Copy/Move uses "from". */
    if ((opcode == MOVE) || (opcode == COPY)) {
        sbJSON *from = get_object_item(patch, "from");
        if (from == NULL) {
            /* missing "from" for copy/move. */
            status = 4;
            goto cleanup;
        }

        if (opcode == MOVE) {
            value = detach_path(object, (unsigned char *)from->u.valuestring);
        }
        if (opcode == COPY) {
            value = get_item_from_pointer(object, from->u.valuestring);
        }
        if (value == NULL) {
            /* missing "from" for copy/move. */
            status = 5;
            goto cleanup;
        }
        if (opcode == COPY) {
            value = sbJSON_Duplicate(value, 1);
        }
        if (value == NULL) {
            /* out of memory for copy/move. */
            status = 6;
            goto cleanup;
        }
    } else /* Add/Replace uses "value". */
    {
        value = get_object_item(patch, "value");
        if (value == NULL) {
            /* missing "value" for add/replace. */
            status = 7;
            goto cleanup;
        }
        value = sbJSON_Duplicate(value, 1);
        if (value == NULL) {
            /* out of memory for add/replace. */
            status = 8;
            goto cleanup;
        }
    }

    /* Now, just add "value" to "path". */

    /* split pointer in parent and child */
    parent_pointer = sbJSONUtils_strdup((unsigned char *)path->u.valuestring);
    if (parent_pointer) {
        child_pointer = (unsigned char *)strrchr((char *)parent_pointer, '/');
    }
    if (child_pointer != NULL) {
        child_pointer[0] = '\0';
        child_pointer++;
    }
    parent =
        get_item_from_pointer(object, (char *)parent_pointer);
    decode_pointer_inplace(child_pointer);

    /* add, remove, replace, move, copy, test. */
    if ((parent == NULL) || (child_pointer == NULL)) {
        /* Couldn't find object to add to. */
        status = 9;
        goto cleanup;
    } else if (sbJSON_IsArray(parent)) {
        if (strcmp((char *)child_pointer, "-") == 0) {
            sbJSON_AddItemToArray(parent, value);
            value = NULL;
        } else {
            size_t index = 0;
            if (!decode_array_index_from_pointer(child_pointer, &index)) {
                status = 11;
                goto cleanup;
            }

            if (!insert_item_in_array(parent, index, value)) {
                status = 10;
                goto cleanup;
            }
            value = NULL;
        }
    } else if (sbJSON_IsObject(parent)) {
        sbJSON_DeleteItemFromObject(parent, (char *)child_pointer);
        sbJSON_AddItemToObject(parent, (char *)child_pointer, value);
        value = NULL;
    } else /* parent is not an object */
    {
        /* Couldn't find object to add to. */
        status = 9;
        goto cleanup;
    }

cleanup:
    if (value != NULL) {
        sbJSON_Delete(value);
    }
    if (parent_pointer != NULL) {
        sbJSON_free(parent_pointer);
    }

    return status;
}

int sbJSONUtils_ApplyPatches(sbJSON *const object,
                             const sbJSON *const patches) {
    const sbJSON *current_patch = NULL;
    int status = 0;

    if (!sbJSON_IsArray(patches)) {
        /* malformed patches. */
        return 1;
    }

    if (patches != NULL) {
        current_patch = patches->child;
    }

    while (current_patch != NULL) {
        status = apply_patch(object, current_patch);
        if (status != 0) {
            return status;
        }
        current_patch = current_patch->next;
    }

    return 0;
}

static void compose_patch(sbJSON *const patches,
                          const unsigned char *const operation,
                          const unsigned char *const path,
                          const unsigned char *suffix,
                          const sbJSON *const value) {
    sbJSON *patch = NULL;

    if ((patches == NULL) || (operation == NULL) || (path == NULL)) {
        return;
    }

    patch = sbJSON_CreateObject();
    if (patch == NULL) {
        return;
    }
    sbJSON_AddItemToObject(patch, "op",
                           sbJSON_CreateString((const char *)operation));

    if (suffix == NULL) {
        sbJSON_AddItemToObject(patch, "path",
                               sbJSON_CreateString((const char *)path));
    } else {
        size_t suffix_length = pointer_encoded_length(suffix);
        size_t path_length = strlen((const char *)path);
        unsigned char *full_path = (unsigned char *)sbJSON_malloc(
            path_length + suffix_length + sizeof("/"));

        sprintf((char *)full_path, "%s/", (const char *)path);
        encode_string_as_pointer(full_path + path_length + 1, suffix);

        sbJSON_AddItemToObject(patch, "path",
                               sbJSON_CreateString((const char *)full_path));
        sbJSON_free(full_path);
    }

    if (value != NULL) {
        sbJSON_AddItemToObject(patch, "value", sbJSON_Duplicate(value, 1));
    }
    sbJSON_AddItemToArray(patches, patch);
}

void sbJSONUtils_AddPatchToArray(sbJSON *const array,
                                 const char *const operation,
                                 const char *const path,
                                 const sbJSON *const value) {
    compose_patch(array, (const unsigned char *)operation,
                  (const unsigned char *)path, NULL, value);
}

static void create_patches(sbJSON *const patches,
                           const unsigned char *const path, sbJSON *const from,
                           sbJSON *const to) {
    if ((from == NULL) || (to == NULL)) {
        return;
    }

    if (from->type != to->type) {
        compose_patch(patches, (const unsigned char *)"replace", path, 0, to);
        return;
    }

    switch (from->type) {
    case sbJSON_Number:
        if (!numbers_match(from, to)) {
            compose_patch(patches, (const unsigned char *)"replace", path, NULL,
                          to);
        }
        return;

    case sbJSON_String:
        if (strcmp(from->u.valuestring, to->u.valuestring) != 0) {
            compose_patch(patches, (const unsigned char *)"replace", path, NULL,
                          to);
        }
        return;

    case sbJSON_Array: {
        size_t index = 0;
        sbJSON *from_child = from->child;
        sbJSON *to_child = to->child;
        unsigned char *new_path = (unsigned char *)sbJSON_malloc(
            strlen((const char *)path) + 20 +
            sizeof("/")); /* Allow space for 64bit int. log10(2^64) = 20 */

        /* generate patches for all array elements that exist in both "from" and
         * "to" */
        for (index = 0; (from_child != NULL) && (to_child != NULL);
             (void)(from_child = from_child->next),
            (void)(to_child = to_child->next), index++) {
            /* check if conversion to unsigned long is valid
             * This should be eliminated at compile time by dead code
             * elimination if size_t is an alias of unsigned long, or if it is
             * bigger */
            if (index > ULONG_MAX) {
                sbJSON_free(new_path);
                return;
            }
            sprintf(
                (char *)new_path, "%s/%lu", path,
                (unsigned long)index); /* path of the current array element */
            create_patches(patches, new_path, from_child, to_child);
        }

        /* remove leftover elements from 'from' that are not in 'to' */
        for (; (from_child != NULL); (void)(from_child = from_child->next)) {
            /* check if conversion to unsigned long is valid
             * This should be eliminated at compile time by dead code
             * elimination if size_t is an alias of unsigned long, or if it is
             * bigger */
            if (index > ULONG_MAX) {
                sbJSON_free(new_path);
                return;
            }
            sprintf((char *)new_path, "%lu", (unsigned long)index);
            compose_patch(patches, (const unsigned char *)"remove", path,
                          new_path, NULL);
        }
        /* add new elements in 'to' that were not in 'from' */
        for (; (to_child != NULL); (void)(to_child = to_child->next), index++) {
            compose_patch(patches, (const unsigned char *)"add", path,
                          (const unsigned char *)"-", to_child);
        }
        sbJSON_free(new_path);
        return;
    }

    case sbJSON_Object: {
        sbJSON *from_child = NULL;
        sbJSON *to_child = NULL;
        sort_object(from);
        sort_object(to);

        from_child = from->child;
        to_child = to->child;
        /* for all object values in the object with more of them */
        while ((from_child != NULL) || (to_child != NULL)) {
            int diff;
            if (from_child == NULL) {
                diff = 1;
            } else if (to_child == NULL) {
                diff = -1;
            } else {
                diff = compare_strings((unsigned char *)from_child->string,
                                       (unsigned char *)to_child->string);
            }

            if (diff == 0) {
                /* both object keys are the same */
                size_t path_length = strlen((const char *)path);
                size_t from_child_name_length =
                    pointer_encoded_length((unsigned char *)from_child->string);
                unsigned char *new_path = (unsigned char *)sbJSON_malloc(
                    path_length + from_child_name_length + sizeof("/"));

                sprintf((char *)new_path, "%s/", path);
                encode_string_as_pointer(new_path + path_length + 1,
                                         (unsigned char *)from_child->string);

                /* create a patch for the element */
                create_patches(patches, new_path, from_child, to_child);
                sbJSON_free(new_path);

                from_child = from_child->next;
                to_child = to_child->next;
            } else if (diff < 0) {
                /* object element doesn't exist in 'to' --> remove it */
                compose_patch(patches, (const unsigned char *)"remove", path,
                              (unsigned char *)from_child->string, NULL);

                from_child = from_child->next;
            } else {
                /* object element doesn't exist in 'from' --> add it */
                compose_patch(patches, (const unsigned char *)"add", path,
                              (unsigned char *)to_child->string, to_child);

                to_child = to_child->next;
            }
        }
        return;
    }

    default:
        break;
    }
}

sbJSON *sbJSONUtils_GeneratePatches(sbJSON *const from, sbJSON *const to) {
    sbJSON *patches = NULL;

    if ((from == NULL) || (to == NULL)) {
        return NULL;
    }

    patches = sbJSON_CreateArray();
    create_patches(patches, (const unsigned char *)"", from, to);

    return patches;
}

void sbJSONUtils_SortObject(sbJSON *const object) {
    sort_object(object);
}

static sbJSON *merge_patch(sbJSON *target, const sbJSON *const patch) {
    sbJSON *patch_child = NULL;

    if (!sbJSON_IsObject(patch)) {
        /* scalar value, array or NULL, just duplicate */
        sbJSON_Delete(target);
        return sbJSON_Duplicate(patch, 1);
    }

    if (!sbJSON_IsObject(target)) {
        sbJSON_Delete(target);
        target = sbJSON_CreateObject();
    }

    patch_child = patch->child;
    while (patch_child != NULL) {
        if (sbJSON_IsNull(patch_child)) {
            /* NULL is the indicator to remove a value, see RFC7396 */
            sbJSON_DeleteItemFromObject(target, patch_child->string);
        } else {
            sbJSON *replace_me = NULL;
            sbJSON *replacement = NULL;

            replace_me =
                sbJSON_DetachItemFromObject(target, patch_child->string);

            replacement = merge_patch(replace_me, patch_child);
            if (replacement == NULL) {
                sbJSON_Delete(target);
                return NULL;
            }

            sbJSON_AddItemToObject(target, patch_child->string, replacement);
        }
        patch_child = patch_child->next;
    }
    return target;
}

sbJSON *sbJSONUtils_MergePatch(sbJSON *target, const sbJSON *const patch) {
    return merge_patch(target, patch);
}

static sbJSON *generate_merge_patch(sbJSON *const from, sbJSON *const to) {
    sbJSON *from_child = NULL;
    sbJSON *to_child = NULL;
    sbJSON *patch = NULL;
    if (to == NULL) {
        /* patch to delete everything */
        return sbJSON_CreateNull();
    }
    if (!sbJSON_IsObject(to) || !sbJSON_IsObject(from)) {
        return sbJSON_Duplicate(to, 1);
    }

    sort_object(from);
    sort_object(to);

    from_child = from->child;
    to_child = to->child;
    patch = sbJSON_CreateObject();
    if (patch == NULL) {
        return NULL;
    }
    while (from_child || to_child) {
        int diff;
        if (from_child != NULL) {
            if (to_child != NULL) {
                diff = strcmp(from_child->string, to_child->string);
            } else {
                diff = -1;
            }
        } else {
            diff = 1;
        }

        if (diff < 0) {
            /* from has a value that to doesn't have -> remove */
            sbJSON_AddItemToObject(patch, from_child->string,
                                   sbJSON_CreateNull());

            from_child = from_child->next;
        } else if (diff > 0) {
            /* to has a value that from doesn't have -> add to patch */
            sbJSON_AddItemToObject(patch, to_child->string,
                                   sbJSON_Duplicate(to_child, 1));

            to_child = to_child->next;
        } else {
            /* object key exists in both objects */
            if (!compare_json(from_child, to_child)) {
                /* not identical --> generate a patch */
                sbJSON_AddItemToObject(
                    patch, to_child->string,
                    sbJSONUtils_GenerateMergePatch(from_child, to_child));
            }

            /* next key in the object */
            from_child = from_child->next;
            to_child = to_child->next;
        }
    }
    if (patch->child == NULL) {
        /* no patch generated */
        sbJSON_Delete(patch);
        return NULL;
    }

    return patch;
}

sbJSON *sbJSONUtils_GenerateMergePatch(sbJSON *const from, sbJSON *const to) {
    return generate_merge_patch(from, to);
}
