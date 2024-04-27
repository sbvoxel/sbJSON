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
    out = sbJSON_Print(root);

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
    if (!sbJSON_PrintPreallocated(root, buf, (int)len, 1)) {
        printf("sbJSON_PrintPreallocated failed!\n");
        if (strcmp(out, buf) != 0) {
            printf("sbJSON_PrintPreallocated not the same as sbJSON_Print!\n");
            printf("sbJSON_Print result:\n%s\n", out);
            printf("sbJSON_PrintPreallocated result:\n%s\n", buf);
        }
        free(out);
        free(buf_fail);
        free(buf);
        return -1;
    }

    /* success */
    printf("%s\n", buf);

    /* force it to fail */
    if (sbJSON_PrintPreallocated(root, buf_fail, (int)len_fail, 1)) {
        printf("sbJSON_PrintPreallocated failed to show error with insufficient "
               "memory!\n");
        printf("sbJSON_Print result:\n%s\n", out);
        printf("sbJSON_PrintPreallocated result:\n%s\n", buf_fail);
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
    root = sbJSON_CreateObject();
    sbJSON_AddItemToObject(root, "name",
                          sbJSON_CreateString("Jack (\"Bee\") Nimble"));
    sbJSON_AddItemToObject(root, "format", fmt = sbJSON_CreateObject());
    sbJSON_AddStringToObject(fmt, "type", "rect");
    sbJSON_AddIntegerNumberToObject(fmt, "width", 1920);
    sbJSON_AddIntegerNumberToObject(fmt, "height", 1080);
    sbJSON_AddFalseToObject(fmt, "interlace");
    sbJSON_AddIntegerNumberToObject(fmt, "frame rate", 24);

    /* Print to text */
    if (print_preallocated(root) != 0) {
        sbJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    sbJSON_Delete(root);

    /* Our "days of the week" array: */
    root = sbJSON_CreateStringArray(strings, 7);

    if (print_preallocated(root) != 0) {
        sbJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    sbJSON_Delete(root);

    /* Our matrix: */
    root = sbJSON_CreateArray();
    for (i = 0; i < 3; i++) {
        sbJSON_AddItemToArray(root, sbJSON_CreateIntArray(numbers[i], 3));
    }

    /* sbJSON_ReplaceItemInArray(root, 1, sbJSON_CreateString("Replacement")); */

    if (print_preallocated(root) != 0) {
        sbJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    sbJSON_Delete(root);

    /* Our "gallery" item: */
    root = sbJSON_CreateObject();
    sbJSON_AddItemToObject(root, "Image", img = sbJSON_CreateObject());
    sbJSON_AddIntegerNumberToObject(img, "Width", 800);
    sbJSON_AddIntegerNumberToObject(img, "Height", 600);
    sbJSON_AddStringToObject(img, "Title", "View from 15th Floor");
    sbJSON_AddItemToObject(img, "Thumbnail", thm = sbJSON_CreateObject());
    sbJSON_AddStringToObject(thm, "Url",
                            "http:/*www.example.com/image/481989943");
    sbJSON_AddIntegerNumberToObject(thm, "Height", 125);
    sbJSON_AddStringToObject(thm, "Width", "100");
    sbJSON_AddItemToObject(img, "IDs", sbJSON_CreateIntArray(ids, 4));

    if (print_preallocated(root) != 0) {
        sbJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    sbJSON_Delete(root);

    /* Our array of "records": */
    root = sbJSON_CreateArray();
    for (i = 0; i < 2; i++) {
        sbJSON_AddItemToArray(root, fld = sbJSON_CreateObject());
        sbJSON_AddStringToObject(fld, "precision", fields[i].precision);
        sbJSON_AddDoubleNumberToObject(fld, "Latitude", fields[i].lat);
        sbJSON_AddDoubleNumberToObject(fld, "Longitude", fields[i].lon);
        sbJSON_AddStringToObject(fld, "Address", fields[i].address);
        sbJSON_AddStringToObject(fld, "City", fields[i].city);
        sbJSON_AddStringToObject(fld, "State", fields[i].state);
        sbJSON_AddStringToObject(fld, "Zip", fields[i].zip);
        sbJSON_AddStringToObject(fld, "Country", fields[i].country);
    }

    /* sbJSON_ReplaceItemInObject(sbJSON_GetArrayItem(root, 1), "City",
     * sbJSON_CreateIntArray(ids, 4)); */

    if (print_preallocated(root) != 0) {
        sbJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    sbJSON_Delete(root);

    root = sbJSON_CreateObject();
    sbJSON_AddDoubleNumberToObject(root, "number", 1.0 / zero);

    if (print_preallocated(root) != 0) {
        sbJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    sbJSON_Delete(root);
}

int main(void) {
    // some sample code
    create_objects();
    return 0;
}
