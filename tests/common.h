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

#ifndef CJSON_TESTS_COMMON_H
#define CJSON_TESTS_COMMON_H

#include "../sbJSON.c"

void reset(sbJSON *item);
void reset(sbJSON *item) {
    if ((item != NULL) && (item->child != NULL)) {
        sbJSON_Delete(item->child);
    }

    if (!item->is_reference && (item->type == sbJSON_String || item->type == sbJSON_Raw)) {
        global_hooks.deallocate(item->u.valuestring);
    }

    if ((!item->string_is_const) && (item->string != NULL)) {
        global_hooks.deallocate(item->string);
    }

    memset(item, 0, sizeof(sbJSON));
}

char *read_file(const char *filename);
char *read_file(const char *filename) {
    FILE *file = NULL;
    long length = 0;
    char *content = NULL;
    size_t read_chars = 0;

    /* open in read binary mode */
    file = fopen(filename, "rb");
    if (file == NULL) {
        goto cleanup;
    }

    /* get the length */
    if (fseek(file, 0, SEEK_END) != 0) {
        goto cleanup;
    }
    length = ftell(file);
    if (length < 0) {
        goto cleanup;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        goto cleanup;
    }

    /* allocate content buffer */
    content = (char *)malloc((size_t)length + sizeof(""));
    if (content == NULL) {
        goto cleanup;
    }

    /* read the file into memory */
    read_chars = fread(content, sizeof(char), (size_t)length, file);
    if ((long)read_chars != length) {
        free(content);
        content = NULL;
        goto cleanup;
    }
    content[read_chars] = '\0';

cleanup:
    if (file != NULL) {
        fclose(file);
    }

    return content;
}

/* assertion helper macros */
#define assert_has_type(item, item_type)                                       \
    TEST_ASSERT_EQUAL_MESSAGE(item_type, item->type,                           \
                             "Item doesn't have expected type.")
#define assert_has_no_reference(item)                                          \
    TEST_ASSERT_FALSE_MESSAGE(item->is_reference,                              \
                             "Item should not have a string as reference.")
#define assert_has_no_const_string(item)                                       \
    TEST_ASSERT_FALSE_MESSAGE(item->string_is_const,                           \
                             "Item should not have a const string.")
#define assert_has_valuestring(item)                                           \
    TEST_ASSERT_TRUE_MESSAGE((item->type == sbJSON_String || item->type == sbJSON_Raw) && item->u.valuestring != NULL, "Valuestring is NULL.")
#define assert_has_no_valuestring(item)                                        \
    TEST_ASSERT_FALSE_MESSAGE((item->type == sbJSON_String || item->type == sbJSON_Raw) && item->u.valuestring != NULL, "Valuestring is not NULL.")
#define assert_has_string(item)                                                \
    TEST_ASSERT_NOT_NULL_MESSAGE(item->string, "String is NULL")
#define assert_has_no_string(item)                                             \
    TEST_ASSERT_NULL_MESSAGE(item->string, "String is not NULL.")
#define assert_not_in_list(item)                                               \
    TEST_ASSERT_NULL_MESSAGE(item->next,                                       \
                             "Linked list next pointer is not NULL.");         \
    TEST_ASSERT_NULL_MESSAGE(item->prev,                                       \
                             "Linked list previous pointer is not NULL.")
#define assert_has_child(item)                                                 \
    TEST_ASSERT_NOT_NULL_MESSAGE(item->child, "Item doesn't have a child.")
#define assert_has_no_child(item)                                              \
    TEST_ASSERT_NULL_MESSAGE(item->child, "Item has a child.")
#define assert_is_invalid(item)                                                \
    assert_has_type(item, sbJSON_Invalid);                                      \
    assert_not_in_list(item);                                                  \
    assert_has_no_child(item);                                                 \
    assert_has_no_string(item);                                                \
    assert_has_no_valuestring(item)

#endif
