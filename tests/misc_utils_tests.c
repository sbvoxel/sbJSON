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

#include "../sbJSON_Utils.h"
#include "common.h"
#include "unity.h"

static void sbjson_utils_functions_shouldnt_crash_with_null_pointers(void) {
    sbJSON *item = sbJSON_CreateString("item");
    TEST_ASSERT_NOT_NULL(item);

    TEST_ASSERT_NULL(sbJSONUtils_GetPointer(item, NULL));
    TEST_ASSERT_NULL(sbJSONUtils_GetPointer(NULL, "pointer"));
    TEST_ASSERT_NULL(sbJSONUtils_GetPointerCaseSensitive(NULL, "pointer"));
    TEST_ASSERT_NULL(sbJSONUtils_GetPointerCaseSensitive(item, NULL));
    TEST_ASSERT_NULL(sbJSONUtils_GeneratePatches(item, NULL));
    TEST_ASSERT_NULL(sbJSONUtils_GeneratePatches(NULL, item));
    TEST_ASSERT_NULL(sbJSONUtils_GeneratePatchesCaseSensitive(item, NULL));
    TEST_ASSERT_NULL(sbJSONUtils_GeneratePatchesCaseSensitive(NULL, item));
    sbJSONUtils_AddPatchToArray(item, "path", "add", NULL);
    sbJSONUtils_AddPatchToArray(item, "path", NULL, item);
    sbJSONUtils_AddPatchToArray(item, NULL, "add", item);
    sbJSONUtils_AddPatchToArray(NULL, "path", "add", item);
    sbJSONUtils_ApplyPatches(item, NULL);
    sbJSONUtils_ApplyPatches(NULL, item);
    sbJSONUtils_ApplyPatchesCaseSensitive(item, NULL);
    sbJSONUtils_ApplyPatchesCaseSensitive(NULL, item);
    TEST_ASSERT_NULL(sbJSONUtils_MergePatch(item, NULL));
    item = sbJSON_CreateString("item");
    TEST_ASSERT_NULL(sbJSONUtils_MergePatchCaseSensitive(item, NULL));
    item = sbJSON_CreateString("item");
    /* these calls are actually valid */
    /* sbJSONUtils_MergePatch(NULL, item); */
    /* sbJSONUtils_MergePatchCaseSensitive(NULL, item);*/
    /* sbJSONUtils_GenerateMergePatch(item, NULL); */
    /* sbJSONUtils_GenerateMergePatch(NULL, item); */
    /* sbJSONUtils_GenerateMergePatchCaseSensitive(item, NULL); */
    /* sbJSONUtils_GenerateMergePatchCaseSensitive(NULL, item); */

    TEST_ASSERT_NULL(sbJSONUtils_FindPointerFromObjectTo(item, NULL));
    TEST_ASSERT_NULL(sbJSONUtils_FindPointerFromObjectTo(NULL, item));
    sbJSONUtils_SortObject(NULL);
    sbJSONUtils_SortObjectCaseSensitive(NULL);

    sbJSON_Delete(item);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(sbjson_utils_functions_shouldnt_crash_with_null_pointers);

    return UNITY_END();
}
