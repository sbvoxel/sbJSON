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

/* sbJSON */
/* JSON parser in C. */

#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sbJSON.h"

typedef struct {
    unsigned char const *json;
    size_t position;
} error;
static error global_error = {NULL, 0};

char const *sbJSON_GetErrorPtr(void) {
    return (char const *)(global_error.json + global_error.position);
}

char *sbj_get_string_value(sbJSON const *const item) {
    if (item == NULL) {
        return NULL;
    }

    assert(sbj_is_string(item));
    return item->u.valuestring;
}

char *sbj_try_get_string_value(sbJSON const *const item) {
    if (item == NULL || item->type != sbJSON_String) {
        return NULL;
    }

    return item->u.valuestring;
}

// TODO: Is this API smart anymore (int vs double)?
double sbj_get_number_value(sbJSON const *const item) {
    if (item == NULL) {
        return NAN;
    }

    assert(item->type == sbJSON_Number);

    if (item->is_number_double) {
        return item->u.valuedouble;
    } else {
        return (double)item->u.valueint;
    }
}

double sbj_try_get_number_value(sbJSON const *const item) {
    if (item == NULL || item->type != sbJSON_Number) {
        return NAN;
    }

    if (item->is_number_double) {
        return item->u.valuedouble;
    } else {
        return (double)item->u.valueint;
    }
}

bool sbj_get_bool_value(sbJSON const *const item) {
    if (item == NULL) {
        return false; // ...
    }

    assert(item->type == sbJSON_Bool);
    return item->u.valuebool;
}

bool sbj_try_get_bool_value(sbJSON const *const item, bool default_bool) {
    if (item == NULL) {
        return default_bool;
    }

    if (item->type == sbJSON_Bool) {
        return item->u.valuebool;
    }

    return default_bool;
}

typedef struct internal_hooks {
    void *(*allocate)(size_t size);
    void (*deallocate)(void *pointer);
    void *(*reallocate)(void *pointer, size_t size);
} internal_hooks;

#if defined(_MSC_VER)
/* work around MSVC error C2322: '...' address of dllimport '...' is not static
 */
static void *internal_malloc(size_t size) { return malloc(size); }
static void internal_free(void *pointer) { free(pointer); }
static void *internal_realloc(void *pointer, size_t size) {
    return realloc(pointer, size);
}
#else
#define internal_malloc malloc
#define internal_free free
#define internal_realloc realloc
#endif

/* strlen of character literals resolved at compile time */
#define static_strlen(string_literal) (sizeof(string_literal) - sizeof(""))

static internal_hooks global_hooks = {internal_malloc, internal_free,
                                      internal_realloc};

static unsigned char *sbJSON_strdup(unsigned char const *string,
                                    internal_hooks const *const hooks) {
    size_t length = 0;
    unsigned char *copy = NULL;

    if (string == NULL) {
        return NULL;
    }

    length = strlen((char const *)string) + sizeof("");
    copy = (unsigned char *)hooks->allocate(length);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, string, length);

    return copy;
}

void sbJSON_InitHooks(sbJSON_Hooks *hooks) {
    if (hooks == NULL) {
        /* Reset hooks */
        global_hooks.allocate = malloc;
        global_hooks.deallocate = free;
        global_hooks.reallocate = realloc;
        return;
    }

    global_hooks.allocate = malloc;
    if (hooks->malloc_fn != NULL) {
        global_hooks.allocate = hooks->malloc_fn;
    }

    global_hooks.deallocate = free;
    if (hooks->free_fn != NULL) {
        global_hooks.deallocate = hooks->free_fn;
    }

    /* use realloc only if both free and malloc are used */
    global_hooks.reallocate = NULL;
    if ((global_hooks.allocate == malloc) &&
        (global_hooks.deallocate == free)) {
        global_hooks.reallocate = realloc;
    }
}

/* Internal constructor. */
static sbJSON *sbJSON_New_Item(internal_hooks const *const hooks) {
    sbJSON *node = (sbJSON *)hooks->allocate(sizeof(sbJSON));
    if (node) {
        memset(node, '\0', sizeof(sbJSON));
    }

    return node;
}

/* Delete a sbJSON structure. */
void sbj_delete(sbJSON *item) {
    sbJSON *next = NULL;
    while (item != NULL) {
        next = item->next;
        if ((!item->is_reference) && (item->child != NULL)) {
            sbj_delete(item->child);
        }

        if (!item->is_reference &&
            (item->type == sbJSON_String || item->type == sbJSON_Raw)) {
            global_hooks.deallocate(item->u.valuestring);
        }

        if ((!item->string_is_const) && (item->string != NULL)) {
            global_hooks.deallocate(item->string);
        }

        global_hooks.deallocate(item);
        item = next;
    }
}

typedef struct {
    unsigned char const *content;
    size_t length;
    size_t offset;
    size_t depth; /* How deeply nested (in arrays/objects) is the input at the
                     current offset. */
    internal_hooks hooks;
} parse_buffer;

/* check if the given size is left to read in a given parse buffer (starting
 * with 1) */
#define can_read(buffer, size)                                                 \
    ((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))
/* check if the buffer can be accessed at the given index (starting with 0) */
#define can_access_at_index(buffer, index)                                     \
    ((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
#define cannot_access_at_index(buffer, index)                                  \
    (!can_access_at_index(buffer, index))
/* get a pointer to the buffer at the position */
#define buffer_at_offset(buffer) ((buffer)->content + (buffer)->offset)

/* Parse the input text to generate a number, and populate the result into item.
 */
static bool parse_number(sbJSON *const item, parse_buffer *const input_buffer) {
    unsigned char number_c_string[64];
    size_t i = 0;

    if ((input_buffer == NULL) || (input_buffer->content == NULL)) {
        return false;
    }

    bool decimal_number = false;

    unsigned char const *buffer = buffer_at_offset(input_buffer);
    const size_t buffer_length = input_buffer->length - input_buffer->offset;

    /* Copy the number into a temporary buffer. This takes care of '\0' not
     * necessarily being available for marking the end of the input */
    for (i = 0; (i < (sizeof(number_c_string) - 1)) && i < buffer_length; i++) {
        unsigned char const c = buffer[i];

        switch (c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '+':
        case '-':
            number_c_string[i] = c;
            break;
        case 'e':
        case 'E':
        case '.':
            number_c_string[i] = c;
            decimal_number = true;
            break;

        default:
            goto loop_end;
        }
    }
loop_end:
    number_c_string[i] = '\0';

    unsigned char *after_end = NULL;

    if (!decimal_number) {
        int64_t number =
            strtoll((char const *)number_c_string, (char **)&after_end, 10);
        if (number_c_string == after_end) {
            return false; /* parse_error */
        }

        // TODO: When a number is exactly LLONG_MAX or LLONG_MIN,
        // it will be classified as a double which is a bug!
        if (number == LLONG_MAX) {
            // Number too large to be stored as an integer.
            after_end = NULL;
            decimal_number = true;
        } else if (number == LLONG_MIN) {
            after_end = NULL;
            decimal_number = true;
        } else {
            item->type = sbJSON_Number;
            sbj_set_integer_number_value(item, number);
        }
    }

    if (decimal_number) {
        double number =
            strtod((char const *)number_c_string, (char **)&after_end);
        if (number_c_string == after_end) {
            return false; /* parse_error */
        }
        item->type = sbJSON_Number;
        sbj_set_double_number_value(item, number);
    }

    input_buffer->offset += (size_t)(after_end - number_c_string);
    return true;
}

void sbj_set_double_number_value(sbJSON *object, double number) {
    if (object == NULL) {
        return;
    }

    assert(object->type == sbJSON_Number);
    object->u.valuedouble = number;
    object->is_number_double = true;
}

void sbj_set_integer_number_value(sbJSON *object, int64_t number) {
    if (object == NULL) {
        return;
    }

    assert(object->type == sbJSON_Number);
    object->u.valueint = number;
    object->is_number_double = false;
}

char *sbj_set_valuestring(sbJSON *object, char const *valuestring) {
    if (object == NULL) {
        return NULL;
    }

    assert(object->type == sbJSON_String && !object->is_reference &&
           valuestring != NULL);

    if (strlen(valuestring) <= strlen(object->u.valuestring)) {
        strcpy(object->u.valuestring, valuestring);
        return object->u.valuestring;
    }

    char *copy = (char *)sbJSON_strdup((unsigned char const *)valuestring,
                                       &global_hooks);
    if (copy == NULL) {
        return NULL;
    }

    sbJSON_free(object->u.valuestring);
    object->u.valuestring = copy;
    return copy;
}

bool sbj_set_bool_value(sbJSON *object, bool boolValue) {
    if (object == NULL) {
        return false;
    }

    if (object->type != sbJSON_Bool) {
        return false;
    }

    assert(!object->is_reference && !object->string_is_const);
    object->type = sbJSON_Bool;
    object->u.valuebool = boolValue;
    return true;
}

typedef struct {
    unsigned char *buffer;
    size_t length;
    size_t offset;
    size_t depth; /* current nesting depth (for formatted printing) */
    bool noalloc;
    bool format; /* is this print a formatted print */
    internal_hooks hooks;
} printbuffer;

/* realloc printbuffer if necessary to have at least "needed" bytes more */
static unsigned char *ensure(printbuffer *const p, size_t needed) {
    unsigned char *newbuffer = NULL;
    size_t newsize = 0;

    if ((p == NULL) || (p->buffer == NULL)) {
        return NULL;
    }

    if ((p->length > 0) && (p->offset >= p->length)) {
        /* make sure that offset is valid */
        return NULL;
    }

    if (needed > INT_MAX) {
        /* sizes bigger than INT_MAX are currently not supported */
        return NULL;
    }

    needed += p->offset + 1;
    if (needed <= p->length) {
        return p->buffer + p->offset;
    }

    if (p->noalloc) {
        return NULL;
    }

    /* calculate new buffer size */
    if (needed > (INT_MAX / 2)) {
        /* overflow of int, use INT_MAX if possible */
        if (needed <= INT_MAX) {
            newsize = INT_MAX;
        } else {
            return NULL;
        }
    } else {
        newsize = needed * 2;
    }

    if (p->hooks.reallocate != NULL) {
        /* reallocate with realloc if available */
        newbuffer = (unsigned char *)p->hooks.reallocate(p->buffer, newsize);
        if (newbuffer == NULL) {
            p->hooks.deallocate(p->buffer);
            p->length = 0;
            p->buffer = NULL;

            return NULL;
        }
    } else {
        /* otherwise reallocate manually */
        newbuffer = (unsigned char *)p->hooks.allocate(newsize);
        if (!newbuffer) {
            p->hooks.deallocate(p->buffer);
            p->length = 0;
            p->buffer = NULL;

            return NULL;
        }

        memcpy(newbuffer, p->buffer, p->offset + 1);
        p->hooks.deallocate(p->buffer);
    }
    p->length = newsize;
    p->buffer = newbuffer;

    return newbuffer + p->offset;
}

/* calculate the new length of the string in a printbuffer and update the offset
 */
static void update_offset(printbuffer *const buffer) {
    unsigned char const *buffer_pointer = NULL;
    if (buffer->buffer == NULL) {
        return;
    }
    buffer_pointer = buffer->buffer + buffer->offset;

    buffer->offset += strlen((char const *)buffer_pointer);
}

/* securely comparison of floating-point variables */
static bool compare_double(double a, double b) {
    double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
    return (fabs(a - b) <= maxVal * DBL_EPSILON);
}

/* Render the number nicely from the given item into a string. */
static bool print_number(sbJSON const *const item,
                         printbuffer *const output_buffer) {

    unsigned char *output_pointer = NULL;
    int length = 0;
    size_t i = 0;
    unsigned char number_buffer[26] = {
        0}; /* temporary buffer to print the number into */
    double test = 0.0;

    if (output_buffer == NULL) {
        return false;
    }

    if (item->is_number_double) {
        double d = item->u.valuedouble;

        /* This checks for NaN and Infinity */
        if (isnan(d) || isinf(d)) {
            // TODO: This behavior should be controllable. Crashing is at least
            // as valid. Also think about rountrip ability.
            length = sprintf((char *)number_buffer, "null");
        } else if (d == (int64_t)d) {
            length = sprintf((char *)number_buffer, "%" PRId64, (int64_t)d);
        } else {
            /* Try 15 decimal places of precision to avoid nonsignificant
             * nonzero digits */
            length = sprintf((char *)number_buffer, "%1.15g", d);

            /* Check whether the original double can be recovered */
            if ((sscanf((char *)number_buffer, "%lg", &test) != 1) ||
                !compare_double((double)test, d)) {
                /* If not, print with 17 decimal places of precision */
                length = sprintf((char *)number_buffer, "%1.17g", d);
            }
        }
    } else {
        int64_t i = item->u.valueint;
        length = sprintf((char *)number_buffer, "%" PRId64, i);
    }

    /* sprintf failed or buffer overrun occurred */
    if ((length < 0) || (length > (int)(sizeof(number_buffer) - 1))) {
        return false;
    }

    /* reserve appropriate space in the output */
    output_pointer = ensure(output_buffer, (size_t)length + sizeof(""));
    if (output_pointer == NULL) {
        return false;
    }

    for (i = 0; i < ((size_t)length); i++) {
        output_pointer[i] = number_buffer[i];
    }
    output_pointer[i] = '\0';

    output_buffer->offset += (size_t)length;

    return true;
}

/* parse 4 digit hexadecimal number */
static unsigned parse_hex4(unsigned char const *const input) {
    unsigned int h = 0;
    size_t i = 0;

    for (i = 0; i < 4; i++) {
        /* parse digit */
        if ((input[i] >= '0') && (input[i] <= '9')) {
            h += (unsigned int)input[i] - '0';
        } else if ((input[i] >= 'A') && (input[i] <= 'F')) {
            h += (unsigned int)10 + input[i] - 'A';
        } else if ((input[i] >= 'a') && (input[i] <= 'f')) {
            h += (unsigned int)10 + input[i] - 'a';
        } else /* invalid */
        {
            return 0;
        }

        if (i < 3) {
            /* shift left to make place for the next nibble */
            h = h << 4;
        }
    }

    return h;
}

/* converts a UTF-16 literal to UTF-8
 * A literal can be one or two sequences of the form \uXXXX */
static unsigned char
utf16_literal_to_utf8(unsigned char const *const input_pointer,
                      unsigned char const *const input_end,
                      unsigned char **output_pointer) {
    long unsigned int codepoint = 0;
    unsigned int first_code = 0;
    unsigned char const *first_sequence = input_pointer;
    unsigned char utf8_length = 0;
    unsigned char utf8_position = 0;
    unsigned char sequence_length = 0;
    unsigned char first_byte_mark = 0;

    if ((input_end - first_sequence) < 6) {
        /* input ends unexpectedly */
        goto fail;
    }

    /* get the first utf16 sequence */
    first_code = parse_hex4(first_sequence + 2);

    /* check that the code is valid */
    if (((first_code >= 0xDC00) && (first_code <= 0xDFFF))) {
        goto fail;
    }

    /* UTF16 surrogate pair */
    if ((first_code >= 0xD800) && (first_code <= 0xDBFF)) {
        unsigned char const *second_sequence = first_sequence + 6;
        unsigned int second_code = 0;
        sequence_length = 12; /* \uXXXX\uXXXX */

        if ((input_end - second_sequence) < 6) {
            /* input ends unexpectedly */
            goto fail;
        }

        if ((second_sequence[0] != '\\') || (second_sequence[1] != 'u')) {
            /* missing second half of the surrogate pair */
            goto fail;
        }

        /* get the second utf16 sequence */
        second_code = parse_hex4(second_sequence + 2);
        /* check that the code is valid */
        if ((second_code < 0xDC00) || (second_code > 0xDFFF)) {
            /* invalid second half of the surrogate pair */
            goto fail;
        }

        /* calculate the unicode codepoint from the surrogate pair */
        codepoint =
            0x10000 + (((first_code & 0x3FF) << 10) | (second_code & 0x3FF));
    } else {
        sequence_length = 6; /* \uXXXX */
        codepoint = first_code;
    }

    /* encode as UTF-8
     * takes at maximum 4 bytes to encode:
     * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
    if (codepoint < 0x80) {
        /* normal ascii, encoding 0xxxxxxx */
        utf8_length = 1;
    } else if (codepoint < 0x800) {
        /* two bytes, encoding 110xxxxx 10xxxxxx */
        utf8_length = 2;
        first_byte_mark = 0xC0; /* 11000000 */
    } else if (codepoint < 0x10000) {
        /* three bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx */
        utf8_length = 3;
        first_byte_mark = 0xE0; /* 11100000 */
    } else if (codepoint <= 0x10FFFF) {
        /* four bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx 10xxxxxx */
        utf8_length = 4;
        first_byte_mark = 0xF0; /* 11110000 */
    } else {
        /* invalid unicode codepoint */
        goto fail;
    }

    /* encode as utf8 */
    for (utf8_position = (unsigned char)(utf8_length - 1); utf8_position > 0;
         utf8_position--) {
        /* 10xxxxxx */
        (*output_pointer)[utf8_position] =
            (unsigned char)((codepoint | 0x80) & 0xBF);
        codepoint >>= 6;
    }
    /* encode first byte */
    if (utf8_length > 1) {
        (*output_pointer)[0] =
            (unsigned char)((codepoint | first_byte_mark) & 0xFF);
    } else {
        (*output_pointer)[0] = (unsigned char)(codepoint & 0x7F);
    }

    *output_pointer += utf8_length;

    return sequence_length;

fail:
    return 0;
}

/* Parse the input text into an unescaped cinput, and populate item. */
static bool parse_string(sbJSON *const item, parse_buffer *const input_buffer) {
    unsigned char const *input_pointer = buffer_at_offset(input_buffer) + 1;
    unsigned char const *input_end = buffer_at_offset(input_buffer) + 1;
    unsigned char *output_pointer = NULL;
    unsigned char *output = NULL;

    /* not a string */
    if (buffer_at_offset(input_buffer)[0] != '\"') {
        goto fail;
    }

    {
        /* calculate approximate size of the output (overestimate) */
        size_t allocation_length = 0;
        size_t skipped_bytes = 0;
        while (((size_t)(input_end - input_buffer->content) <
                input_buffer->length) &&
               (*input_end != '\"')) {
            /* is escape sequence */
            if (input_end[0] == '\\') {
                if ((size_t)(input_end + 1 - input_buffer->content) >=
                    input_buffer->length) {
                    /* prevent buffer overflow when last input character is a
                     * backslash */
                    goto fail;
                }
                skipped_bytes++;
                input_end++;
            }
            input_end++;
        }
        if (((size_t)(input_end - input_buffer->content) >=
             input_buffer->length) ||
            (*input_end != '\"')) {
            goto fail; /* string ended unexpectedly */
        }

        /* This is at most how much we need for the output */
        allocation_length =
            (size_t)(input_end - buffer_at_offset(input_buffer)) -
            skipped_bytes;
        output = (unsigned char *)input_buffer->hooks.allocate(
            allocation_length + sizeof(""));
        if (output == NULL) {
            goto fail; /* allocation failure */
        }
    }

    output_pointer = output;
    /* loop through the string literal */
    while (input_pointer < input_end) {
        if (*input_pointer != '\\') {
            *output_pointer++ = *input_pointer++;
        }
        /* escape sequence */
        else {
            unsigned char sequence_length = 2;
            if ((input_end - input_pointer) < 1) {
                goto fail;
            }

            switch (input_pointer[1]) {
            case 'b':
                *output_pointer++ = '\b';
                break;
            case 'f':
                *output_pointer++ = '\f';
                break;
            case 'n':
                *output_pointer++ = '\n';
                break;
            case 'r':
                *output_pointer++ = '\r';
                break;
            case 't':
                *output_pointer++ = '\t';
                break;
            case '\"':
            case '\\':
            case '/':
                *output_pointer++ = input_pointer[1];
                break;

            /* UTF-16 literal */
            case 'u':
                sequence_length = utf16_literal_to_utf8(
                    input_pointer, input_end, &output_pointer);
                if (sequence_length == 0) {
                    /* failed to convert UTF16-literal to UTF-8 */
                    goto fail;
                }
                break;

            default:
                goto fail;
            }
            input_pointer += sequence_length;
        }
    }

    /* zero terminate the output */
    *output_pointer = '\0';

    item->type = sbJSON_String;
    item->u.valuestring = (char *)output;

    input_buffer->offset = (size_t)(input_end - input_buffer->content);
    input_buffer->offset++;

    return true;

fail:
    if (output != NULL) {
        input_buffer->hooks.deallocate(output);
    }

    if (input_pointer != NULL) {
        input_buffer->offset = (size_t)(input_pointer - input_buffer->content);
    }

    return false;
}

/* Render the cstring provided to an escaped version that can be printed. */
static bool print_string_ptr(unsigned char const *const input,
                             printbuffer *const output_buffer) {
    unsigned char const *input_pointer = NULL;
    unsigned char *output = NULL;
    unsigned char *output_pointer = NULL;
    size_t output_length = 0;
    /* numbers of additional characters needed for escaping */
    size_t escape_characters = 0;

    if (output_buffer == NULL) {
        return false;
    }

    /* empty string */
    if (input == NULL) {
        output = ensure(output_buffer, sizeof("\"\""));
        if (output == NULL) {
            return false;
        }
        strcpy((char *)output, "\"\"");

        return true;
    }

    /* set "flag" to 1 if something needs to be escaped */
    for (input_pointer = input; *input_pointer; input_pointer++) {
        switch (*input_pointer) {
        case '\"':
        case '\\':
        case '\b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
            /* one character escape sequence */
            escape_characters++;
            break;
        default:
            if (*input_pointer < 32) {
                /* UTF-16 escape sequence uXXXX */
                escape_characters += 5;
            }
            break;
        }
    }
    output_length = (size_t)(input_pointer - input) + escape_characters;

    output = ensure(output_buffer, output_length + sizeof("\"\""));
    if (output == NULL) {
        return false;
    }

    /* no characters have to be escaped */
    if (escape_characters == 0) {
        output[0] = '\"';
        memcpy(output + 1, input, output_length);
        output[output_length + 1] = '\"';
        output[output_length + 2] = '\0';

        return true;
    }

    output[0] = '\"';
    output_pointer = output + 1;
    /* copy the string */
    for (input_pointer = input; *input_pointer != '\0';
         (void)input_pointer++, output_pointer++) {
        if ((*input_pointer > 31) && (*input_pointer != '\"') &&
            (*input_pointer != '\\')) {
            /* normal character, copy */
            *output_pointer = *input_pointer;
        } else {
            /* character needs to be escaped */
            *output_pointer++ = '\\';
            switch (*input_pointer) {
            case '\\':
                *output_pointer = '\\';
                break;
            case '\"':
                *output_pointer = '\"';
                break;
            case '\b':
                *output_pointer = 'b';
                break;
            case '\f':
                *output_pointer = 'f';
                break;
            case '\n':
                *output_pointer = 'n';
                break;
            case '\r':
                *output_pointer = 'r';
                break;
            case '\t':
                *output_pointer = 't';
                break;
            default:
                /* escape and print as unicode codepoint */
                sprintf((char *)output_pointer, "u%04x", *input_pointer);
                output_pointer += 4;
                break;
            }
        }
    }
    output[output_length + 1] = '\"';
    output[output_length + 2] = '\0';

    return true;
}

/* Invoke print_string_ptr (which is useful) on an item. */
static bool print_string(sbJSON const *const item, printbuffer *const p) {
    return print_string_ptr((unsigned char *)item->u.valuestring, p);
}

/* Predeclare these prototypes. */
static bool parse_value(sbJSON *const item, parse_buffer *const input_buffer);
static bool print_value(sbJSON const *const item,
                        printbuffer *const output_buffer);
static bool parse_array(sbJSON *const item, parse_buffer *const input_buffer);
static bool print_array(sbJSON const *const item,
                        printbuffer *const output_buffer);
static bool parse_object(sbJSON *const item, parse_buffer *const input_buffer);
static bool print_object(sbJSON const *const item,
                         printbuffer *const output_buffer);

/* Utility to jump whitespace and cr/lf */
static parse_buffer *buffer_skip_whitespace(parse_buffer *const buffer) {
    if ((buffer == NULL) || (buffer->content == NULL)) {
        return NULL;
    }

    if (cannot_access_at_index(buffer, 0)) {
        return buffer;
    }

    while (can_access_at_index(buffer, 0) &&
           (buffer_at_offset(buffer)[0] <= 32)) {
        buffer->offset++;
    }

    if (buffer->offset == buffer->length) {
        buffer->offset--;
    }

    return buffer;
}

/* skip the UTF-8 BOM (byte order mark) if it is at the beginning of a buffer */
static parse_buffer *skip_utf8_bom(parse_buffer *const buffer) {
    if ((buffer == NULL) || (buffer->content == NULL) ||
        (buffer->offset != 0)) {
        return NULL;
    }

    if (can_access_at_index(buffer, 4) &&
        (strncmp((char const *)buffer_at_offset(buffer), "\xEF\xBB\xBF", 3) ==
         0)) {
        buffer->offset += 3;
    }

    return buffer;
}

sbJSON *sbj_parse_with_opts(char const *value, char const **return_parse_end,
                             bool require_null_terminated) {
    size_t buffer_length;

    if (NULL == value) {
        return NULL;
    }

    /* Adding null character size due to require_null_terminated. */
    buffer_length = strlen(value) + sizeof("");

    return sbj_parse_with_length_opts(value, buffer_length, return_parse_end,
                                      require_null_terminated);
}

/* Parse an object - create a new root, and populate. */
sbJSON *sbj_parse_with_length_opts(char const *value, size_t buffer_length,
                                   char const **return_parse_end,
                                   bool require_null_terminated) {
    parse_buffer buffer = {0, 0, 0, 0, {0, 0, 0}};
    sbJSON *item = NULL;

    /* reset error position */
    global_error.json = NULL;
    global_error.position = 0;

    if (value == NULL || 0 == buffer_length) {
        goto fail;
    }

    buffer.content = (unsigned char const *)value;
    buffer.length = buffer_length;
    buffer.offset = 0;
    buffer.hooks = global_hooks;

    item = sbJSON_New_Item(&global_hooks);
    if (item == NULL) /* memory fail */
    {
        goto fail;
    }

    if (!parse_value(item, buffer_skip_whitespace(skip_utf8_bom(&buffer)))) {
        /* parse failure. ep is set. */
        goto fail;
    }

    /* if we require null-terminated JSON without appended garbage, skip and
     * then check for a null terminator */
    if (require_null_terminated) {
        buffer_skip_whitespace(&buffer);
        if ((buffer.offset >= buffer.length) ||
            buffer_at_offset(&buffer)[0] != '\0') {
            goto fail;
        }
    }
    if (return_parse_end) {
        *return_parse_end = (char const *)buffer_at_offset(&buffer);
    }

    return item;

fail:
    if (item != NULL) {
        sbj_delete(item);
    }

    if (value != NULL) {
        error local_error;
        local_error.json = (unsigned char const *)value;
        local_error.position = 0;

        if (buffer.offset < buffer.length) {
            local_error.position = buffer.offset;
        } else if (buffer.length > 0) {
            local_error.position = buffer.length - 1;
        }

        if (return_parse_end != NULL) {
            *return_parse_end =
                (char const *)local_error.json + local_error.position;
        }

        global_error = local_error;
    }

    return NULL;
}

/* Default options for sbj_parse */
sbJSON *sbj_parse(char const *value) {
    return sbj_parse_with_opts(value, 0, 0);
}

sbJSON *sbj_parse_with_length(char const *value, size_t buffer_length) {
    return sbj_parse_with_length_opts(value, buffer_length, 0, 0);
}

#define sbjson_min(a, b) (((a) < (b)) ? (a) : (b))

static unsigned char *print(sbJSON const *const item, bool format,
                            internal_hooks const *const hooks) {
    static const size_t default_buffer_size = 256;
    printbuffer buffer[1];
    unsigned char *printed = NULL;

    memset(buffer, 0, sizeof(buffer));

    /* create buffer */
    buffer->buffer = (unsigned char *)hooks->allocate(default_buffer_size);
    buffer->length = default_buffer_size;
    buffer->format = format;
    buffer->hooks = *hooks;
    if (buffer->buffer == NULL) {
        goto fail;
    }

    /* print the value */
    if (!print_value(item, buffer)) {
        goto fail;
    }
    update_offset(buffer);

    /* check if reallocate is available */
    if (hooks->reallocate != NULL) {
        printed = (unsigned char *)hooks->reallocate(buffer->buffer,
                                                     buffer->offset + 1);
        if (printed == NULL) {
            goto fail;
        }
        buffer->buffer = NULL;
    } else /* otherwise copy the JSON over to a new buffer */
    {
        printed = (unsigned char *)hooks->allocate(buffer->offset + 1);
        if (printed == NULL) {
            goto fail;
        }
        memcpy(printed, buffer->buffer,
               sbjson_min(buffer->length, buffer->offset + 1));
        printed[buffer->offset] = '\0'; /* just to be sure */

        /* free the buffer */
        hooks->deallocate(buffer->buffer);
    }

    return printed;

fail:
    if (buffer->buffer != NULL) {
        hooks->deallocate(buffer->buffer);
    }

    if (printed != NULL) {
        hooks->deallocate(printed);
    }

    return NULL;
}

/* Render a sbJSON item/entity/structure to text. */
char *sbj_print(sbJSON const *item) {
    return (char *)print(item, true, &global_hooks);
}

char *sbj_print_unformatted(sbJSON const *item) {
    return (char *)print(item, false, &global_hooks);
}

char *sbj_print_buffered(sbJSON const *item, int prebuffer, bool fmt) {
    printbuffer p = {0, 0, 0, 0, 0, 0, {0, 0, 0}};

    if (prebuffer < 0) {
        return NULL;
    }

    p.buffer = (unsigned char *)global_hooks.allocate((size_t)prebuffer);
    if (!p.buffer) {
        return NULL;
    }

    p.length = (size_t)prebuffer;
    p.offset = 0;
    p.noalloc = false;
    p.format = fmt;
    p.hooks = global_hooks;

    if (!print_value(item, &p)) {
        global_hooks.deallocate(p.buffer);
        return NULL;
    }

    return (char *)p.buffer;
}

bool sbj_print_preallocated(sbJSON *item, char *buffer, int const length,
                              bool const format) {
    printbuffer p = {0, 0, 0, 0, 0, 0, {0, 0, 0}};

    if ((length < 0) || (buffer == NULL)) {
        return false;
    }

    p.buffer = (unsigned char *)buffer;
    p.length = (size_t)length;
    p.offset = 0;
    p.noalloc = true;
    p.format = format;
    p.hooks = global_hooks;

    return print_value(item, &p);
}

/* Parser core - when encountering text, process appropriately. */
static bool parse_value(sbJSON *const item, parse_buffer *const input_buffer) {
    if ((input_buffer == NULL) || (input_buffer->content == NULL)) {
        return false; /* no input */
    }

    /* parse the different types of values */
    /* null */
    if (can_read(input_buffer, 4) &&
        (strncmp((char const *)buffer_at_offset(input_buffer), "null", 4) ==
         0)) {
        item->type = sbJSON_Null;
        input_buffer->offset += 4;
        return true;
    }
    /* false */
    if (can_read(input_buffer, 5) &&
        (strncmp((char const *)buffer_at_offset(input_buffer), "false", 5) ==
         0)) {
        item->type = sbJSON_Bool;
        item->u.valuebool = false;
        input_buffer->offset += 5;
        return true;
    }
    /* true */
    if (can_read(input_buffer, 4) &&
        (strncmp((char const *)buffer_at_offset(input_buffer), "true", 4) ==
         0)) {
        item->type = sbJSON_Bool;
        item->u.valuebool = true;
        input_buffer->offset += 4;
        return true;
    }
    /* string */
    if (can_access_at_index(input_buffer, 0) &&
        (buffer_at_offset(input_buffer)[0] == '\"')) {
        return parse_string(item, input_buffer);
    }
    /* number */
    if (can_access_at_index(input_buffer, 0) &&
        ((buffer_at_offset(input_buffer)[0] == '-') ||
         ((buffer_at_offset(input_buffer)[0] >= '0') &&
          (buffer_at_offset(input_buffer)[0] <= '9')))) {
        return parse_number(item, input_buffer);
    }
    /* array */
    if (can_access_at_index(input_buffer, 0) &&
        (buffer_at_offset(input_buffer)[0] == '[')) {
        return parse_array(item, input_buffer);
    }
    /* object */
    if (can_access_at_index(input_buffer, 0) &&
        (buffer_at_offset(input_buffer)[0] == '{')) {
        return parse_object(item, input_buffer);
    }

    return false;
}

/* Render a value to text. */
static bool print_value(sbJSON const *const item,
                        printbuffer *const output_buffer) {
    unsigned char *output = NULL;

    if ((item == NULL) || (output_buffer == NULL)) {
        return false;
    }

    switch (item->type) {
    case sbJSON_Null:
        output = ensure(output_buffer, 5);
        if (output == NULL) {
            return false;
        }
        strcpy((char *)output, "null");
        return true;
    case sbJSON_Bool:
        if (item->u.valuebool) {
            output = ensure(output_buffer, 5);
            if (output == NULL) {
                return false;
            }
            strcpy((char *)output, "true");
            return true;
        } else {
            output = ensure(output_buffer, 6);
            if (output == NULL) {
                return false;
            }
            strcpy((char *)output, "false");
            return true;
        }
    case sbJSON_Number:
        return print_number(item, output_buffer);
    case sbJSON_Raw: {
        size_t raw_length = 0;
        if (item->u.valuestring == NULL) {
            return false;
        }

        raw_length = strlen(item->u.valuestring) + sizeof("");
        output = ensure(output_buffer, raw_length);
        if (output == NULL) {
            return false;
        }
        memcpy(output, item->u.valuestring, raw_length);
        return true;
    }
    case sbJSON_String:
        return print_string(item, output_buffer);
    case sbJSON_Array:
        return print_array(item, output_buffer);
    case sbJSON_Object:
        return print_object(item, output_buffer);
    default:
        return false;
    }
}

/* Build an array from input text. */
static bool parse_array(sbJSON *const item, parse_buffer *const input_buffer) {
    sbJSON *head = NULL; /* head of the linked list */
    sbJSON *current_item = NULL;

    if (input_buffer->depth >= SBJSON_NESTING_LIMIT) {
        return false; /* to deeply nested */
    }
    input_buffer->depth++;

    if (buffer_at_offset(input_buffer)[0] != '[') {
        /* not an array */
        goto fail;
    }

    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    if (can_access_at_index(input_buffer, 0) &&
        (buffer_at_offset(input_buffer)[0] == ']')) {
        /* empty array */
        goto success;
    }

    /* check if we skipped to the end of the buffer */
    if (cannot_access_at_index(input_buffer, 0)) {
        input_buffer->offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    input_buffer->offset--;
    /* loop through the comma separated array elements */
    do {
        /* allocate next item */
        sbJSON *new_item = sbJSON_New_Item(&(input_buffer->hooks));
        if (new_item == NULL) {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL) {
            /* start the linked list */
            current_item = head = new_item;
        } else {
            /* add to the end and advance */
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        /* parse next value */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_value(current_item, input_buffer)) {
            goto fail; /* failed to parse value */
        }
        buffer_skip_whitespace(input_buffer);
    } while (can_access_at_index(input_buffer, 0) &&
             (buffer_at_offset(input_buffer)[0] == ','));

    if (cannot_access_at_index(input_buffer, 0) ||
        buffer_at_offset(input_buffer)[0] != ']') {
        goto fail; /* expected end of array */
    }

success:
    input_buffer->depth--;

    if (head != NULL) {
        head->prev = current_item;
    }

    item->type = sbJSON_Array;
    item->child = head;

    input_buffer->offset++;

    return true;

fail:
    if (head != NULL) {
        sbj_delete(head);
    }

    return false;
}

/* Render an array to text */
static bool print_array(sbJSON const *const item,
                        printbuffer *const output_buffer) {
    unsigned char *output_pointer = NULL;
    size_t length = 0;
    sbJSON *current_element = item->child;

    /* Compose the output array. */
    /* opening square bracket */
    output_pointer = ensure(output_buffer, 1);
    if (output_pointer == NULL) {
        return false;
    }

    *output_pointer = '[';
    output_buffer->offset++;
    output_buffer->depth++;

    while (current_element != NULL) {
        if (!print_value(current_element, output_buffer)) {
            return false;
        }
        update_offset(output_buffer);
        if (current_element->next) {
            length = (size_t)(output_buffer->format ? 2 : 1);
            output_pointer = ensure(output_buffer, length + 1);
            if (output_pointer == NULL) {
                return false;
            }
            *output_pointer++ = ',';
            if (output_buffer->format) {
                *output_pointer++ = ' ';
            }
            *output_pointer = '\0';
            output_buffer->offset += length;
        }
        current_element = current_element->next;
    }

    output_pointer = ensure(output_buffer, 2);
    if (output_pointer == NULL) {
        return false;
    }
    *output_pointer++ = ']';
    *output_pointer = '\0';
    output_buffer->depth--;

    return true;
}

/* Build an object from the text. */
static bool parse_object(sbJSON *const item, parse_buffer *const input_buffer) {
    sbJSON *head = NULL; /* linked list head */
    sbJSON *current_item = NULL;

    if (input_buffer->depth >= SBJSON_NESTING_LIMIT) {
        return false; /* to deeply nested */
    }
    input_buffer->depth++;

    if (cannot_access_at_index(input_buffer, 0) ||
        (buffer_at_offset(input_buffer)[0] != '{')) {
        goto fail; /* not an object */
    }

    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    if (can_access_at_index(input_buffer, 0) &&
        (buffer_at_offset(input_buffer)[0] == '}')) {
        goto success; /* empty object */
    }

    /* check if we skipped to the end of the buffer */
    if (cannot_access_at_index(input_buffer, 0)) {
        input_buffer->offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    input_buffer->offset--;
    /* loop through the comma separated array elements */
    do {
        /* allocate next item */
        sbJSON *new_item = sbJSON_New_Item(&(input_buffer->hooks));
        if (new_item == NULL) {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL) {
            /* start the linked list */
            current_item = head = new_item;
        } else {
            /* add to the end and advance */
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        if (input_buffer->offset + 1 >= input_buffer->length) {
            goto fail; /* nothing comes after the comma */
        }

        /* parse the name of the child */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_string(current_item, input_buffer)) {
            goto fail; /* failed to parse name */
        }
        buffer_skip_whitespace(input_buffer);

        /* swap valuestring and string, because we parsed the name */
        current_item->string = current_item->u.valuestring;
        current_item->u.valuestring = NULL;

        if (cannot_access_at_index(input_buffer, 0) ||
            (buffer_at_offset(input_buffer)[0] != ':')) {
            goto fail; /* invalid object */
        }

        /* parse the value */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_value(current_item, input_buffer)) {
            goto fail; /* failed to parse value */
        }
        buffer_skip_whitespace(input_buffer);
    } while (can_access_at_index(input_buffer, 0) &&
             (buffer_at_offset(input_buffer)[0] == ','));

    if (cannot_access_at_index(input_buffer, 0) ||
        (buffer_at_offset(input_buffer)[0] != '}')) {
        goto fail; /* expected end of object */
    }

success:
    input_buffer->depth--;

    if (head != NULL) {
        head->prev = current_item;
    }

    item->type = sbJSON_Object;
    item->child = head;

    input_buffer->offset++;
    return true;

fail:
    if (head != NULL) {
        sbj_delete(head);
    }

    return false;
}

/* Render an object to text. */
static bool print_object(sbJSON const *const item,
                         printbuffer *const output_buffer) {
    unsigned char *output_pointer = NULL;
    size_t length = 0;
    sbJSON *current_item = item->child;

    if (output_buffer == NULL) {
        return false;
    }

    /* Compose the output: */
    length = (size_t)(output_buffer->format ? 2 : 1); /* fmt: {\n */
    output_pointer = ensure(output_buffer, length + 1);
    if (output_pointer == NULL) {
        return false;
    }

    *output_pointer++ = '{';
    output_buffer->depth++;
    if (output_buffer->format) {
        *output_pointer++ = '\n';
    }
    output_buffer->offset += length;

    while (current_item) {
        if (output_buffer->format) {
            size_t i;
            output_pointer = ensure(output_buffer, output_buffer->depth);
            if (output_pointer == NULL) {
                return false;
            }
            for (i = 0; i < output_buffer->depth; i++) {
                *output_pointer++ = '\t';
            }
            output_buffer->offset += output_buffer->depth;
        }

        /* print key */
        if (!print_string_ptr((unsigned char *)current_item->string,
                              output_buffer)) {
            return false;
        }
        update_offset(output_buffer);

        length = (size_t)(output_buffer->format ? 2 : 1);
        output_pointer = ensure(output_buffer, length);
        if (output_pointer == NULL) {
            return false;
        }
        *output_pointer++ = ':';
        if (output_buffer->format) {
            *output_pointer++ = '\t';
        }
        output_buffer->offset += length;

        /* print value */
        if (!print_value(current_item, output_buffer)) {
            return false;
        }
        update_offset(output_buffer);

        /* print comma if not last */
        length = ((size_t)(output_buffer->format ? 1 : 0) +
                  (size_t)(current_item->next ? 1 : 0));
        output_pointer = ensure(output_buffer, length + 1);
        if (output_pointer == NULL) {
            return false;
        }
        if (current_item->next) {
            *output_pointer++ = ',';
        }

        if (output_buffer->format) {
            *output_pointer++ = '\n';
        }
        *output_pointer = '\0';
        output_buffer->offset += length;

        current_item = current_item->next;
    }

    output_pointer = ensure(
        output_buffer, output_buffer->format ? (output_buffer->depth + 1) : 2);
    if (output_pointer == NULL) {
        return false;
    }
    if (output_buffer->format) {
        size_t i;
        for (i = 0; i < (output_buffer->depth - 1); i++) {
            *output_pointer++ = '\t';
        }
    }
    *output_pointer++ = '}';
    *output_pointer = '\0';
    output_buffer->depth--;

    return true;
}

/* Get Array size/item / object item. */
int sbj_get_array_size(sbJSON const *array) {
    sbJSON *child = NULL;
    size_t size = 0;

    if (array == NULL) {
        return 0;
    }

    child = array->child;

    while (child != NULL) {
        size++;
        child = child->next;
    }

    /* FIXME: Can overflow here. Cannot be fixed without breaking the API */

    return (int)size;
}

static sbJSON *get_array_item(sbJSON const *array, size_t index) {
    sbJSON *current_child = NULL;

    if (array == NULL) {
        return NULL;
    }

    current_child = array->child;
    while ((current_child != NULL) && (index > 0)) {
        index--;
        current_child = current_child->next;
    }

    return current_child;
}

sbJSON *sbj_get_array_item(sbJSON const *array, int index) {
    if (index < 0) {
        return NULL;
    }

    return get_array_item(array, (size_t)index);
}

static sbJSON *get_object_item(sbJSON const *const object,
                               char const *const name) {
    sbJSON *current_element = NULL;

    if ((object == NULL) || (name == NULL)) {
        return NULL;
    }

    current_element = object->child;
    while ((current_element != NULL) && (current_element->string != NULL) &&
           (strcmp(name, current_element->string) != 0)) {
        current_element = current_element->next;
    }

    if ((current_element == NULL) || (current_element->string == NULL)) {
        return NULL;
    }

    return current_element;
}

sbJSON *sbj_get_object_item(sbJSON const *const object,
                             char const *const string) {
    return get_object_item(object, string);
}

bool sbj_has_object_item(sbJSON const *object, char const *string) {
    return sbj_get_object_item(object, string) ? 1 : 0;
}

/* Utility for array list handling. */
static void suffix_object(sbJSON *prev, sbJSON *item) {
    prev->next = item;
    item->prev = prev;
}

/* Utility for handling references. */
static sbJSON *create_reference(sbJSON const *item,
                                internal_hooks const *const hooks) {
    sbJSON *reference = NULL;
    if (item == NULL) {
        return NULL;
    }

    reference = sbJSON_New_Item(hooks);
    if (reference == NULL) {
        return NULL;
    }

    memcpy(reference, item, sizeof(sbJSON));
    reference->string = NULL;
    reference->is_reference = true;
    reference->next = reference->prev = NULL;
    return reference;
}

static bool add_item_to_array(sbJSON *array, sbJSON *item) {
    sbJSON *child = NULL;

    if (array == NULL || item == NULL) {
        return false;
    }

    assert(array != item);

    child = array->child;
    /*
     * To find the last item in array quickly, we use prev in array
     */
    if (child == NULL) {
        /* list is empty, start new one */
        array->child = item;
        item->prev = item;
        item->next = NULL;
    } else {
        /* append to the end */
        if (child->prev) {
            suffix_object(child->prev, item);
            array->child->prev = item;
        }
    }

    return true;
}

/* Add item to array/object. */
bool sbj_add_item_to_array(sbJSON *array, sbJSON *item) {
    return add_item_to_array(array, item);
}

/* helper function to cast away const */
static void *cast_away_const(void const *string) { return (void *)string; }

static bool add_item_to_object(sbJSON *const object, char const *const string,
                               sbJSON *const item,
                               internal_hooks const *const hooks,
                               bool const constant_key) {
    char *new_key = NULL;
    uint16_t new_type = sbJSON_Invalid;
    bool is_reference = false;
    bool string_is_const = false;

    if (item == NULL) {
        return false;
    }

    assert(object != item);

    if (constant_key) {
        new_key = (char *)cast_away_const(string);
        new_type = item->type;
        string_is_const = true;
    } else {
        new_key = (char *)sbJSON_strdup((unsigned char const *)string, hooks);
        if (new_key == NULL) {
            return false;
        }

        new_type = item->type;
        is_reference = item->is_reference;
        string_is_const = false;
    }

    if ((!item->string_is_const) && (item->string != NULL)) {
        hooks->deallocate(item->string);
    }

    item->string = new_key;
    item->type = new_type;
    item->is_reference = is_reference;
    item->string_is_const = string_is_const;

    return add_item_to_array(object, item);
}

bool sbj_add_item_to_object(sbJSON *object, char const *string, sbJSON *item) {
    return add_item_to_object(object, string, item, &global_hooks, false);
}

/* Add an item to an object with constant string as key */
bool sbj_add_item_to_objectCS(sbJSON *object, char const *string,
                              sbJSON *item) {
    return add_item_to_object(object, string, item, &global_hooks, true);
}

bool sbj_add_item_reference_to_array(sbJSON *array, sbJSON *item) {
    return add_item_to_array(array, create_reference(item, &global_hooks));
}

bool sbj_add_item_reference_to_object(sbJSON *object, char const *string,
                                     sbJSON *item) {
    return add_item_to_object(object, string,
                              create_reference(item, &global_hooks),
                              &global_hooks, false);
}

sbJSON *sbj_add_null_to_object(sbJSON *const object, char const *const name) {
    sbJSON *null = sbj_create_null();
    if (add_item_to_object(object, name, null, &global_hooks, false)) {
        return null;
    }

    sbj_delete(null);
    return NULL;
}

sbJSON *sbj_add_true_to_object(sbJSON *const object, char const *const name) {
    sbJSON *true_item = sbj_create_true();
    if (add_item_to_object(object, name, true_item, &global_hooks, false)) {
        return true_item;
    }

    sbj_delete(true_item);
    return NULL;
}

sbJSON *sbj_add_false_to_object(sbJSON *const object, char const *const name) {
    sbJSON *false_item = sbj_create_false();
    if (add_item_to_object(object, name, false_item, &global_hooks, false)) {
        return false_item;
    }

    sbj_delete(false_item);
    return NULL;
}

sbJSON *sbj_add_bool_to_object(sbJSON *const object, char const *const name,
                               bool const boolean) {
    sbJSON *bool_item = sbj_create_bool(boolean);
    if (add_item_to_object(object, name, bool_item, &global_hooks, false)) {
        return bool_item;
    }

    sbj_delete(bool_item);
    return NULL;
}

sbJSON *sbj_add_double_number_to_object(sbJSON *const object,
                                       char const *const name,
                                       double const number) {
    sbJSON *number_item = sbj_create_double_number(number);
    if (add_item_to_object(object, name, number_item, &global_hooks, false)) {
        return number_item;
    }

    sbj_delete(number_item);
    return NULL;
}

sbJSON *sbj_add_integer_number_to_object(sbJSON *const object,
                                        char const *const name,
                                        const int64_t number) {
    sbJSON *number_item = sbj_create_integer_number(number);
    if (add_item_to_object(object, name, number_item, &global_hooks, false)) {
        return number_item;
    }

    sbj_delete(number_item);
    return NULL;
}

sbJSON *sbj_add_string_to_object(sbJSON *const object, char const *const name,
                                 char const *const string) {
    sbJSON *string_item = sbJSON_CreateString(string);
    if (add_item_to_object(object, name, string_item, &global_hooks, false)) {
        return string_item;
    }

    sbj_delete(string_item);
    return NULL;
}

sbJSON *sbj_add_raw_to_object(sbJSON *const object, char const *const name,
                              char const *const raw) {
    sbJSON *raw_item = sbj_create_raw(raw);
    if (add_item_to_object(object, name, raw_item, &global_hooks, false)) {
        return raw_item;
    }

    sbj_delete(raw_item);
    return NULL;
}

sbJSON *sbj_add_object_to_object(sbJSON *const object, char const *const name) {
    sbJSON *object_item = sbj_create_object();
    if (add_item_to_object(object, name, object_item, &global_hooks, false)) {
        return object_item;
    }

    sbj_delete(object_item);
    return NULL;
}

sbJSON *sbj_add_array_to_object(sbJSON *const object, char const *const name) {
    sbJSON *array = sbj_create_array();
    if (add_item_to_object(object, name, array, &global_hooks, false)) {
        return array;
    }

    sbj_delete(array);
    return NULL;
}

sbJSON *sbj_detach_item_via_pointer(sbJSON *parent, sbJSON *const item) {
    if ((parent == NULL) || (item == NULL)) {
        return NULL;
    }

    if (item != parent->child) {
        /* not the first element */
        item->prev->next = item->next;
    }
    if (item->next != NULL) {
        /* not the last element */
        item->next->prev = item->prev;
    }

    if (item == parent->child) {
        /* first element */
        parent->child = item->next;
    } else if (item->next == NULL) {
        /* last element */
        parent->child->prev = item->prev;
    }

    /* make sure the detached item doesn't point anywhere anymore */
    item->prev = NULL;
    item->next = NULL;

    return item;
}

sbJSON *sbj_detach_item_from_array(sbJSON *array, int which) {
    if (which < 0) {
        return NULL;
    }

    return sbj_detach_item_via_pointer(array,
                                       get_array_item(array, (size_t)which));
}

void sbj_delete_item_from_array(sbJSON *array, int which) {
    sbj_delete(sbj_detach_item_from_array(array, which));
}

sbJSON *sbj_detach_item_from_object(sbJSON *object, char const *string) {
    sbJSON *to_detach = sbj_get_object_item(object, string);

    return sbj_detach_item_via_pointer(object, to_detach);
}

void sbj_delete_item_from_object(sbJSON *object, char const *string) {
    sbj_delete(sbj_detach_item_from_object(object, string));
}

/* Replace array/object items with new ones. */
bool sbj_insert_item_in_array(sbJSON *array, int which, sbJSON *newitem) {
    sbJSON *after_inserted = NULL;

    if (which < 0 || newitem == NULL) {
        return false;
    }

    after_inserted = get_array_item(array, (size_t)which);
    if (after_inserted == NULL) {
        return add_item_to_array(array, newitem);
    }

    if (after_inserted != array->child && after_inserted->prev == NULL) {
        /* return false if after_inserted is a corrupted array item */
        return false;
    }

    newitem->next = after_inserted;
    newitem->prev = after_inserted->prev;
    after_inserted->prev = newitem;
    if (after_inserted == array->child) {
        array->child = newitem;
    } else {
        newitem->prev->next = newitem;
    }
    return true;
}

bool sbj_replace_item_via_pointer(sbJSON *const parent, sbJSON *const item,
                                  sbJSON *replacement) {
    if ((parent == NULL) || (parent->child == NULL) || (replacement == NULL) ||
        (item == NULL)) {
        return false;
    }

    if (replacement == item) {
        return true;
    }

    replacement->next = item->next;
    replacement->prev = item->prev;

    if (replacement->next != NULL) {
        replacement->next->prev = replacement;
    }
    if (parent->child == item) {
        if (parent->child->prev == parent->child) {
            replacement->prev = replacement;
        }
        parent->child = replacement;
    } else { /*
              * To find the last item in array quickly, we use prev in array.
              * We can't modify the last item's next pointer where this item was
              * the parent's child
              */
        if (replacement->prev != NULL) {
            replacement->prev->next = replacement;
        }
        if (replacement->next == NULL) {
            parent->child->prev = replacement;
        }
    }

    item->next = NULL;
    item->prev = NULL;
    sbj_delete(item);

    return true;
}

bool sbj_replace_item_in_array(sbJSON *array, int which, sbJSON *newitem) {
    if (which < 0) {
        return false;
    }

    return sbj_replace_item_via_pointer(
        array, get_array_item(array, (size_t)which), newitem);
}

static bool replace_item_in_object(sbJSON *object, char const *string,
                                   sbJSON *replacement) {
    if ((replacement == NULL) || (string == NULL)) {
        return false;
    }

    /* replace the name in the replacement */
    if ((!replacement->string_is_const) && (replacement->string != NULL)) {
        sbJSON_free(replacement->string);
    }
    replacement->string =
        (char *)sbJSON_strdup((unsigned char const *)string, &global_hooks);
    if (replacement->string == NULL) {
        return false;
    }

    replacement->string_is_const = false;

    return sbj_replace_item_via_pointer(object, get_object_item(object, string),
                                        replacement);
}

bool sbj_replace_item_in_object(sbJSON *object, char const *string,
                                sbJSON *newitem) {
    return replace_item_in_object(object, string, newitem);
}

/* Create basic types: */
sbJSON *sbj_create_null(void) {
    sbJSON *item = sbJSON_New_Item(&global_hooks);
    if (item) {
        item->type = sbJSON_Null;
    }

    return item;
}

sbJSON *sbj_create_true(void) {
    sbJSON *item = sbJSON_New_Item(&global_hooks);
    if (item) {
        item->type = sbJSON_Bool;
        item->u.valuebool = true;
    }

    return item;
}

sbJSON *sbj_create_false(void) {
    sbJSON *item = sbJSON_New_Item(&global_hooks);
    if (item) {
        item->type = sbJSON_Bool;
        item->u.valuebool = false;
    }

    return item;
}

sbJSON *sbj_create_bool(bool boolean) {
    sbJSON *item = sbJSON_New_Item(&global_hooks);
    if (item) {
        item->type = sbJSON_Bool;
        item->u.valuebool = boolean;
    }

    return item;
}

sbJSON *sbj_create_double_number(double num) {
    sbJSON *item = sbJSON_New_Item(&global_hooks);
    if (item) {
        item->type = sbJSON_Number;
        item->u.valuedouble = num;
        item->is_number_double = true;
    }

    return item;
}

sbJSON *sbj_create_integer_number(int64_t num) {
    sbJSON *item = sbJSON_New_Item(&global_hooks);
    if (item) {
        item->type = sbJSON_Number;
        item->u.valueint = num;
        item->is_number_double = false;
    }

    return item;
}

sbJSON *sbJSON_CreateString(char const *string) {
    sbJSON *item = sbJSON_New_Item(&global_hooks);
    if (item) {
        item->type = sbJSON_String;
        item->u.valuestring =
            (char *)sbJSON_strdup((unsigned char const *)string, &global_hooks);
        if (!item->u.valuestring) {
            sbj_delete(item);
            return NULL;
        }
    }

    return item;
}

sbJSON *sbj_create_string_reference(char const *string) {
    sbJSON *item = sbJSON_New_Item(&global_hooks);
    if (item != NULL) {
        item->type = sbJSON_String;
        item->is_reference = true;
        item->u.valuestring = (char *)cast_away_const(string);
    }

    return item;
}

sbJSON *sbj_create_object_reference(sbJSON const *child) {
    sbJSON *item = sbJSON_New_Item(&global_hooks);
    if (item != NULL) {
        item->type = sbJSON_Object;
        item->is_reference = true;
        item->child = (sbJSON *)cast_away_const(child);
    }

    return item;
}

sbJSON *sbj_create_array_reference(sbJSON const *child) {
    sbJSON *item = sbJSON_New_Item(&global_hooks);
    if (item != NULL) {
        item->type = sbJSON_Array;
        item->is_reference = true;
        item->child = (sbJSON *)cast_away_const(child);
    }

    return item;
}

sbJSON *sbj_create_raw(char const *raw) {
    sbJSON *item = sbJSON_New_Item(&global_hooks);
    if (item) {
        item->type = sbJSON_Raw;
        item->u.valuestring =
            (char *)sbJSON_strdup((unsigned char const *)raw, &global_hooks);
        if (!item->u.valuestring) {
            sbj_delete(item);
            return NULL;
        }
    }

    return item;
}

sbJSON *sbj_create_array(void) {
    sbJSON *item = sbJSON_New_Item(&global_hooks);
    if (item) {
        item->type = sbJSON_Array;
    }

    return item;
}

sbJSON *sbj_create_object(void) {
    sbJSON *item = sbJSON_New_Item(&global_hooks);
    if (item) {
        item->type = sbJSON_Object;
    }

    return item;
}

/* Create Arrays: */
sbJSON *sbj_create_int_array(int const *numbers, int count) {
    size_t i = 0;
    sbJSON *n = NULL;
    sbJSON *p = NULL;
    sbJSON *a = NULL;

    if ((count < 0) || (numbers == NULL)) {
        return NULL;
    }

    a = sbj_create_array();

    for (i = 0; a && (i < (size_t)count); i++) {
        n = sbj_create_integer_number(numbers[i]);
        if (!n) {
            sbj_delete(a);
            return NULL;
        }
        if (!i) {
            a->child = n;
        } else {
            suffix_object(p, n);
        }
        p = n;
    }

    if (a && a->child) {
        a->child->prev = n;
    }

    return a;
}

sbJSON *sbj_create_float_array(float const *numbers, int count) {
    size_t i = 0;
    sbJSON *n = NULL;
    sbJSON *p = NULL;
    sbJSON *a = NULL;

    if ((count < 0) || (numbers == NULL)) {
        return NULL;
    }

    a = sbj_create_array();

    for (i = 0; a && (i < (size_t)count); i++) {
        n = sbj_create_double_number((double)numbers[i]);
        if (!n) {
            sbj_delete(a);
            return NULL;
        }
        if (!i) {
            a->child = n;
        } else {
            suffix_object(p, n);
        }
        p = n;
    }

    if (a && a->child) {
        a->child->prev = n;
    }

    return a;
}

sbJSON *sbj_create_double_array(double const *numbers, int count) {
    size_t i = 0;
    sbJSON *n = NULL;
    sbJSON *p = NULL;
    sbJSON *a = NULL;

    if ((count < 0) || (numbers == NULL)) {
        return NULL;
    }

    a = sbj_create_array();

    for (i = 0; a && (i < (size_t)count); i++) {
        n = sbj_create_double_number(numbers[i]);
        if (!n) {
            sbj_delete(a);
            return NULL;
        }
        if (!i) {
            a->child = n;
        } else {
            suffix_object(p, n);
        }
        p = n;
    }

    if (a && a->child) {
        a->child->prev = n;
    }

    return a;
}

sbJSON *sbj_create_string_array(char const *const *strings, int count) {
    size_t i = 0;
    sbJSON *n = NULL;
    sbJSON *p = NULL;
    sbJSON *a = NULL;

    if ((count < 0) || (strings == NULL)) {
        return NULL;
    }

    a = sbj_create_array();

    for (i = 0; a && (i < (size_t)count); i++) {
        n = sbJSON_CreateString(strings[i]);
        if (!n) {
            sbj_delete(a);
            return NULL;
        }
        if (!i) {
            a->child = n;
        } else {
            suffix_object(p, n);
        }
        p = n;
    }

    if (a && a->child) {
        a->child->prev = n;
    }

    return a;
}

/* Duplication */
// TODO: This is missing regular tests (outside of the utils tests)
sbJSON *sbj_duplicate(sbJSON const *item, bool recurse) {
    sbJSON *newitem = NULL;
    sbJSON *child = NULL;
    sbJSON *next = NULL;
    sbJSON *newchild = NULL;

    /* Bail on bad ptr */
    if (!item) {
        goto fail;
    }
    /* Create new item */
    newitem = sbJSON_New_Item(&global_hooks);
    if (!newitem) {
        goto fail;
    }
    /* Copy over all vars */
    newitem->type = item->type;
    newitem->is_reference = false;
    newitem->string_is_const = item->string_is_const;
    newitem->u = item->u;
    newitem->is_number_double = item->is_number_double;

    if (item->type == sbJSON_String || item->type == sbJSON_Raw) {
        newitem->u.valuestring = (char *)sbJSON_strdup(
            (unsigned char *)item->u.valuestring, &global_hooks);
        if (!newitem->u.valuestring) {
            goto fail;
        }
    }

    if (item->string) {
        newitem->string =
            (item->string_is_const)
                ? item->string
                : (char *)sbJSON_strdup((unsigned char *)item->string,
                                        &global_hooks);
        if (!newitem->string) {
            goto fail;
        }
    }

    /* If non-recursive, then we're done! */
    if (!recurse) {
        return newitem;
    }

    /* Walk the ->next chain for the child. */
    child = item->child;
    while (child != NULL) {
        newchild = sbj_duplicate(
            child,
            true); /* Duplicate (with recurse) each item in the ->next chain */
        if (!newchild) {
            goto fail;
        }
        if (next != NULL) {
            /* If newitem->child already set, then crosswire ->prev and ->next
             * and move on */
            next->next = newchild;
            newchild->prev = next;
            next = newchild;
        } else {
            /* Set newitem->child and move to it */
            newitem->child = newchild;
            next = newchild;
        }
        child = child->next;
    }
    if (newitem && newitem->child) {
        newitem->child->prev = newchild;
    }

    return newitem;

fail:
    if (newitem != NULL) {
        sbj_delete(newitem);
    }

    return NULL;
}

static void skip_oneline_comment(char **input) {
    *input += static_strlen("//");

    for (; (*input)[0] != '\0'; ++(*input)) {
        if ((*input)[0] == '\n') {
            *input += static_strlen("\n");
            return;
        }
    }
}

static void skip_multiline_comment(char **input) {
    *input += static_strlen("/*");

    for (; (*input)[0] != '\0'; ++(*input)) {
        if (((*input)[0] == '*') && ((*input)[1] == '/')) {
            *input += static_strlen("*/");
            return;
        }
    }
}

static void minify_string(char **input, char **output) {
    (*output)[0] = (*input)[0];
    *input += static_strlen("\"");
    *output += static_strlen("\"");

    for (; (*input)[0] != '\0'; (void)++(*input), ++(*output)) {
        (*output)[0] = (*input)[0];

        if ((*input)[0] == '\"') {
            (*output)[0] = '\"';
            *input += static_strlen("\"");
            *output += static_strlen("\"");
            return;
        } else if (((*input)[0] == '\\') && ((*input)[1] == '\"')) {
            (*output)[1] = (*input)[1];
            *input += static_strlen("\"");
            *output += static_strlen("\"");
        }
    }
}

void sbj_minify(char *json) {
    char *into = json;

    if (json == NULL) {
        return;
    }

    while (json[0] != '\0') {
        switch (json[0]) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            json++;
            break;
        case '/':
            if (json[1] == '/') {
                skip_oneline_comment(&json);
            } else if (json[1] == '*') {
                skip_multiline_comment(&json);
            } else {
                json++;
            }
            break;
        case '\"':
            minify_string(&json, (char **)&into);
            break;
        default:
            into[0] = json[0];
            json++;
            into++;
        }
    }

    /* and null-terminate. */
    *into = '\0';
}

bool sbj_is_invalid(sbJSON const *const item) {
    if (item == NULL) {
        return false;
    }

    return item->type == sbJSON_Invalid;
}

bool sbj_is_false(sbJSON const *const item) {
    if (item == NULL) {
        return false;
    }

    if (item->type != sbJSON_Bool) {
        return false;
    }

    return !item->u.valuebool;
}

bool sbj_is_true(sbJSON const *const item) {
    if (item == NULL) {
        return false;
    }

    if (item->type != sbJSON_Bool) {
        return false;
    }

    return item->u.valuebool;
}

bool sbj_is_bool(sbJSON const *const item) {
    if (item == NULL) {
        return false;
    }

    return item->type == sbJSON_Bool;
}

bool sbj_is_null(sbJSON const *const item) {
    if (item == NULL) {
        return false;
    }

    return item->type == sbJSON_Null;
}

bool sbj_is_number(sbJSON const *const item) {
    if (item == NULL) {
        return false;
    }

    return item->type == sbJSON_Number;
}

bool sbj_is_string(sbJSON const *const item) {
    if (item == NULL) {
        return false;
    }

    return item->type == sbJSON_String;
}

bool sbj_is_array(sbJSON const *const item) {
    if (item == NULL) {
        return false;
    }

    return item->type == sbJSON_Array;
}

bool sbj_is_object(sbJSON const *const item) {
    if (item == NULL) {
        return false;
    }

    return item->type == sbJSON_Object;
}

bool sbj_is_raw(sbJSON const *const item) {
    if (item == NULL) {
        return false;
    }

    return item->type == sbJSON_Raw;
}

// TODO: https://github.com/DaveGamble/cJSON/issues/748
// Duplicate keys make comparison impossible. Would need
// keys to have an order as well. Starting to be ridiculous.
// Duplicate keys should be opt-in and not parse by default (do they?).
bool sbj_compare(sbJSON const *const a, sbJSON const *const b) {
    if (a == b) {
        return true;
    }

    if (a == NULL || b == NULL) {
        return false;
    }

    if (a->type != b->type) {
        return false;
    }

    switch (a->type) {
    case sbJSON_Invalid:
        return true;
    case sbJSON_Bool:
        return a->u.valuebool == b->u.valuebool;
    case sbJSON_Null:
        return true;
    case sbJSON_Number:
        if (a->is_number_double != b->is_number_double) {
            return false;
        }

        if (a->is_number_double) {
            return compare_double(a->u.valuedouble, b->u.valuedouble);
        } else {
            return a->u.valueint == b->u.valueint;
        }
    case sbJSON_String:
    case sbJSON_Raw:
        if ((a->u.valuestring == NULL) || (b->u.valuestring == NULL)) {
            return false;
        }
        if (strcmp(a->u.valuestring, b->u.valuestring) == 0) {
            return true;
        }

        return false;
    case sbJSON_Array: {
        sbJSON *a_element = a->child;
        sbJSON *b_element = b->child;

        for (; (a_element != NULL) && (b_element != NULL);) {
            if (!sbj_compare(a_element, b_element)) {
                return false;
            }

            a_element = a_element->next;
            b_element = b_element->next;
        }

        /* one of the arrays is longer than the other */
        if (a_element != b_element) {
            return false;
        }

        return true;
    }
    case sbJSON_Object: {
        sbJSON *a_element = NULL;
        sbJSON *b_element = NULL;
        sbJSON_ArrayForEach(a_element, a) {
            /* TODO This has O(n^2) runtime, which is horrible! */
            b_element = get_object_item(b, a_element->string);
            if (b_element == NULL) {
                return false;
            }

            if (!sbj_compare(a_element, b_element)) {
                return false;
            }
        }

        /* doing this twice, once on a and b to prevent true comparison if a
         * subset of b
         * TODO: Do this the proper way, this is just a fix for now */
        sbJSON_ArrayForEach(b_element, b) {
            a_element = get_object_item(a, b_element->string);
            if (a_element == NULL) {
                return false;
            }

            if (!sbj_compare(b_element, a_element)) {
                return false;
            }
        }

        return true;
    }
    default:
        return false;
    }
}

void *sbJSON_malloc(size_t size) { return global_hooks.allocate(size); }

// TODO: Is passing NULL valid?
void sbJSON_free(void *object) { global_hooks.deallocate(object); }
