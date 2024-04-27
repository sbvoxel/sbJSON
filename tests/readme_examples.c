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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "unity.h"

static const char *json = "{\n\
\t\"name\":\t\"Awesome 4K\",\n\
\t\"resolutions\":\t[{\n\
\t\t\t\"width\":\t1280,\n\
\t\t\t\"height\":\t720\n\
\t\t}, {\n\
\t\t\t\"width\":\t1920,\n\
\t\t\t\"height\":\t1080\n\
\t\t}, {\n\
\t\t\t\"width\":\t3840,\n\
\t\t\t\"height\":\t2160\n\
\t\t}]\n\
}";

static char *create_monitor(void) {
    const unsigned int resolution_numbers[3][2] = {
        {1280, 720}, {1920, 1080}, {3840, 2160}};
    char *string = NULL;
    sbJSON *name = NULL;
    sbJSON *resolutions = NULL;
    sbJSON *resolution = NULL;
    sbJSON *width = NULL;
    sbJSON *height = NULL;
    size_t index = 0;

    sbJSON *monitor = sbJSON_CreateObject();
    if (monitor == NULL) {
        goto end;
    }

    name = sbJSON_CreateString("Awesome 4K");
    if (name == NULL) {
        goto end;
    }
    /* after creation was successful, immediately add it to the monitor,
     * thereby transferring ownership of the pointer to it */
    sbJSON_AddItemToObject(monitor, "name", name);

    resolutions = sbJSON_CreateArray();
    if (resolutions == NULL) {
        goto end;
    }
    sbJSON_AddItemToObject(monitor, "resolutions", resolutions);

    for (index = 0; index < (sizeof(resolution_numbers) / (2 * sizeof(int)));
         ++index) {
        resolution = sbJSON_CreateObject();
        if (resolution == NULL) {
            goto end;
        }
        sbJSON_AddItemToArray(resolutions, resolution);

        width = sbJSON_CreateNumber(resolution_numbers[index][0]);
        if (width == NULL) {
            goto end;
        }
        sbJSON_AddItemToObject(resolution, "width", width);

        height = sbJSON_CreateNumber(resolution_numbers[index][1]);
        if (height == NULL) {
            goto end;
        }
        sbJSON_AddItemToObject(resolution, "height", height);
    }

    string = sbJSON_Print(monitor);
    if (string == NULL) {
        fprintf(stderr, "Failed to print monitor.\n");
    }

end:
    sbJSON_Delete(monitor);
    return string;
}

static char *create_monitor_with_helpers(void) {
    const unsigned int resolution_numbers[3][2] = {
        {1280, 720}, {1920, 1080}, {3840, 2160}};
    char *string = NULL;
    sbJSON *resolutions = NULL;
    size_t index = 0;

    sbJSON *monitor = sbJSON_CreateObject();

    if (sbJSON_AddStringToObject(monitor, "name", "Awesome 4K") == NULL) {
        goto end;
    }

    resolutions = sbJSON_AddArrayToObject(monitor, "resolutions");
    if (resolutions == NULL) {
        goto end;
    }

    for (index = 0; index < (sizeof(resolution_numbers) / (2 * sizeof(int)));
         ++index) {
        sbJSON *resolution = sbJSON_CreateObject();

        if (sbJSON_AddNumberToObject(resolution, "width",
                                    resolution_numbers[index][0]) == NULL) {
            goto end;
        }

        if (sbJSON_AddNumberToObject(resolution, "height",
                                    resolution_numbers[index][1]) == NULL) {
            goto end;
        }

        sbJSON_AddItemToArray(resolutions, resolution);
    }

    string = sbJSON_Print(monitor);
    if (string == NULL) {
        fprintf(stderr, "Failed to print monitor.\n");
    }

end:
    sbJSON_Delete(monitor);
    return string;
}

/* return 1 if the monitor supports full hd, 0 otherwise */
static int supports_full_hd(const char *const monitor) {
    const sbJSON *resolution = NULL;
    const sbJSON *resolutions = NULL;
    const sbJSON *name = NULL;
    int status = 0;
    sbJSON *monitor_json = sbJSON_Parse(monitor);
    if (monitor_json == NULL) {
        const char *error_ptr = sbJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        status = 0;
        goto end;
    }

    name = sbJSON_GetObjectItemCaseSensitive(monitor_json, "name");
    if (sbJSON_IsString(name) && (name->valuestring != NULL)) {
        printf("Checking monitor \"%s\"\n", name->valuestring);
    }

    resolutions = sbJSON_GetObjectItemCaseSensitive(monitor_json, "resolutions");
    sbJSON_ArrayForEach(resolution, resolutions) {
        sbJSON *width = sbJSON_GetObjectItemCaseSensitive(resolution, "width");
        sbJSON *height = sbJSON_GetObjectItemCaseSensitive(resolution, "height");

        if (!sbJSON_IsNumber(width) || !sbJSON_IsNumber(height)) {
            status = 0;
            goto end;
        }

        if (compare_double(width->valuedouble, 1920) &&
            compare_double(height->valuedouble, 1080)) {
            status = 1;
            goto end;
        }
    }

end:
    sbJSON_Delete(monitor_json);
    return status;
}

static void create_monitor_should_create_a_monitor(void) {
    char *monitor = create_monitor();

    TEST_ASSERT_EQUAL_STRING(monitor, json);

    free(monitor);
}

static void create_monitor_with_helpers_should_create_a_monitor(void) {
    char *monitor = create_monitor_with_helpers();

    TEST_ASSERT_EQUAL_STRING(json, monitor);

    free(monitor);
}

static void supports_full_hd_should_check_for_full_hd_support(void) {
    static const char *monitor_without_hd = "{\n\
\t\t\"name\": \"lame monitor\",\n\
\t\t\"resolutions\":\t[{\n\
\t\t\t\"width\":\t640,\n\
\t\t\t\"height\":\t480\n\
\t\t}]\n\
}";

    TEST_ASSERT(supports_full_hd(json));
    TEST_ASSERT_FALSE(supports_full_hd(monitor_without_hd));
}

int CJSON_CDECL main(void) {
    UNITY_BEGIN();

    RUN_TEST(create_monitor_should_create_a_monitor);
    RUN_TEST(create_monitor_with_helpers_should_create_a_monitor);
    RUN_TEST(supports_full_hd_should_check_for_full_hd_support);

    return UNITY_END();
}
