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

#include "sbJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Used by some code below as an example datatype. */
struct record {
    const char *precision;
    double lat;
    double lon;
    const char *address;
    const char *city;
    const char *state;
    const char *zip;
    const char *country;
};

/* Create a bunch of objects as demonstration. */
static int print_preallocated(sbJSON *root) {
    /* declarations */
    char *out = NULL;
    char *buf = NULL;
    char *buf_fail = NULL;
    size_t len = 0;
    size_t len_fail = 0;

    /* formatted print */
    out = sbj_print(root);

    /* create buffer to succeed */
    /* the extra 5 bytes are because of inaccuracies when reserving memory */
    len = strlen(out) + 5;
    buf = (char *)malloc(len);
    if (buf == NULL) {
        printf("Failed to allocate memory.\n");
        exit(1);
    }

    /* create buffer to fail */
    len_fail = strlen(out);
    buf_fail = (char *)malloc(len_fail);
    if (buf_fail == NULL) {
        printf("Failed to allocate memory.\n");
        exit(1);
    }

    /* Print to buffer */
    if (!sbj_print_preallocated(root, buf, (int)len, 1)) {
        printf("sbj_print_preallocated failed!\n");
        if (strcmp(out, buf) != 0) {
            printf("sbj_print_preallocated not the same as sbj_print!\n");
            printf("sbj_print result:\n%s\n", out);
            printf("sbj_print_preallocated result:\n%s\n", buf);
        }
        free(out);
        free(buf_fail);
        free(buf);
        return -1;
    }

    /* success */
    printf("%s\n", buf);

    /* force it to fail */
    if (sbj_print_preallocated(root, buf_fail, (int)len_fail, 1)) {
        printf("sbj_print_preallocated failed to show error with insufficient "
               "memory!\n");
        printf("sbj_print result:\n%s\n", out);
        printf("sbj_print_preallocated result:\n%s\n", buf_fail);
        free(out);
        free(buf_fail);
        free(buf);
        return -1;
    }

    free(out);
    free(buf_fail);
    free(buf);
    return 0;
}

/* Create a bunch of objects as demonstration. */
static void create_objects(void) {
    /* declare a few. */
    sbJSON *root = NULL;
    sbJSON *fmt = NULL;
    sbJSON *img = NULL;
    sbJSON *thm = NULL;
    sbJSON *fld = NULL;
    int i = 0;

    /* Our "days of the week" array: */
    const char *strings[7] = {"Sunday",   "Monday", "Tuesday", "Wednesday",
                              "Thursday", "Friday", "Saturday"};
    /* Our matrix: */
    int numbers[3][3] = {{0, -1, 0}, {1, 0, 0}, {0, 0, 1}};
    /* Our "gallery" item: */
    int ids[4] = {116, 943, 234, 38793};
    /* Our array of "records": */
    struct record fields[2] = {
        {"zip", 37.7668, -1.223959e+2, "", "SAN FRANCISCO", "CA", "94107",
         "US"},
        {"zip", 37.371991, -1.22026e+2, "", "SUNNYVALE", "CA", "94085", "US"}};
    volatile double zero = 0.0;

    /* Here we construct some JSON standards, from the JSON site. */

    /* Our "Video" datatype: */
    root = sbj_create_object();
    sbj_add_item_to_object(root, "name",
                          sbJSON_CreateString("Jack (\"Bee\") Nimble"));
    sbj_add_item_to_object(root, "format", fmt = sbj_create_object());
    sbj_add_string_to_object(fmt, "type", "rect");
    sbj_add_integer_number_to_object(fmt, "width", 1920);
    sbj_add_integer_number_to_object(fmt, "height", 1080);
    sbj_add_false_to_object(fmt, "interlace");
    sbj_add_integer_number_to_object(fmt, "frame rate", 24);

    /* Print to text */
    if (print_preallocated(root) != 0) {
        sbj_delete(root);
        exit(EXIT_FAILURE);
    }
    sbj_delete(root);

    /* Our "days of the week" array: */
    root = sbj_create_string_array(strings, 7);

    if (print_preallocated(root) != 0) {
        sbj_delete(root);
        exit(EXIT_FAILURE);
    }
    sbj_delete(root);

    /* Our matrix: */
    root = sbj_create_array();
    for (i = 0; i < 3; i++) {
        sbj_add_item_to_array(root, sbj_create_int_array(numbers[i], 3));
    }

    /* sbj_replace_item_in_array(root, 1, sbJSON_CreateString("Replacement")); */

    if (print_preallocated(root) != 0) {
        sbj_delete(root);
        exit(EXIT_FAILURE);
    }
    sbj_delete(root);

    /* Our "gallery" item: */
    root = sbj_create_object();
    sbj_add_item_to_object(root, "Image", img = sbj_create_object());
    sbj_add_integer_number_to_object(img, "Width", 800);
    sbj_add_integer_number_to_object(img, "Height", 600);
    sbj_add_string_to_object(img, "Title", "View from 15th Floor");
    sbj_add_item_to_object(img, "Thumbnail", thm = sbj_create_object());
    sbj_add_string_to_object(thm, "Url",
                            "http:/*www.example.com/image/481989943");
    sbj_add_integer_number_to_object(thm, "Height", 125);
    sbj_add_string_to_object(thm, "Width", "100");
    sbj_add_item_to_object(img, "IDs", sbj_create_int_array(ids, 4));

    if (print_preallocated(root) != 0) {
        sbj_delete(root);
        exit(EXIT_FAILURE);
    }
    sbj_delete(root);

    /* Our array of "records": */
    root = sbj_create_array();
    for (i = 0; i < 2; i++) {
        sbj_add_item_to_array(root, fld = sbj_create_object());
        sbj_add_string_to_object(fld, "precision", fields[i].precision);
        sbj_add_double_number_to_object(fld, "Latitude", fields[i].lat);
        sbj_add_double_number_to_object(fld, "Longitude", fields[i].lon);
        sbj_add_string_to_object(fld, "Address", fields[i].address);
        sbj_add_string_to_object(fld, "City", fields[i].city);
        sbj_add_string_to_object(fld, "State", fields[i].state);
        sbj_add_string_to_object(fld, "Zip", fields[i].zip);
        sbj_add_string_to_object(fld, "Country", fields[i].country);
    }

    /* sbj_replace_item_in_object(sbj_get_array_item(root, 1), "City",
     * sbj_create_int_array(ids, 4)); */

    if (print_preallocated(root) != 0) {
        sbj_delete(root);
        exit(EXIT_FAILURE);
    }
    sbj_delete(root);

    root = sbj_create_object();
    sbj_add_double_number_to_object(root, "number", 1.0 / zero);

    if (print_preallocated(root) != 0) {
        sbj_delete(root);
        exit(EXIT_FAILURE);
    }
    sbj_delete(root);
}

int main(void) {
    // some sample code
    create_objects();
    return 0;
}
