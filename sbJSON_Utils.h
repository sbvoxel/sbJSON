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

#ifndef sbJSON_Utils__h
#define sbJSON_Utils__h

#ifdef __cplusplus
extern "C" {
#endif

#include "sbJSON.h"

/* Implement RFC6901 (https://tools.ietf.org/html/rfc6901) JSON Pointer spec. */
sbJSON *sbJSONUtils_GetPointer(sbJSON *const object, const char *pointer);

/* Implement RFC6902 (https://tools.ietf.org/html/rfc6902) JSON Patch spec. */
/* NOTE: This modifies objects in 'from' and 'to' by sorting the elements by
 * their key */
sbJSON *sbJSONUtils_GeneratePatches(sbJSON *const from, sbJSON *const to);
/* Utility for generating patch array entries. */
void sbJSONUtils_AddPatchToArray(sbJSON *const array, const char *const operation,
                                const char *const path,
                                const sbJSON *const value);
/* Returns 0 for success. */
int sbJSONUtils_ApplyPatches(sbJSON *const object, const sbJSON *const patches);

/*
// Note that ApplyPatches is NOT atomic on failure. To implement an atomic
ApplyPatches, use:
//int sbJSONUtils_AtomicApplyPatches(sbJSON **object, sbJSON *patches)
//{
//    sbJSON *modme = sbj_duplicate(*object, 1);
//    int error = sbJSONUtils_ApplyPatches(modme, patches);
//    if (!error)
//    {
//        sbj_delete(*object);
//        *object = modme;
//    }
//    else
//    {
//        sbj_delete(modme);
//    }
//
//    return error;
//}
// Code not added to library since this strategy is a LOT slower.
*/

/* Implement RFC7386 (https://tools.ietf.org/html/rfc7396) JSON Merge Patch
 * spec. */
/* target will be modified by patch. return value is new ptr for target. */
sbJSON *sbJSONUtils_MergePatch(sbJSON *target, const sbJSON *const patch);
/* generates a patch to move from -> to */
/* NOTE: This modifies objects in 'from' and 'to' by sorting the elements by
 * their key */
sbJSON *sbJSONUtils_GenerateMergePatch(sbJSON *const from, sbJSON *const to);

/* Given a root object and a target object, construct a pointer from one to the
 * other. */
char *sbJSONUtils_FindPointerFromObjectTo(const sbJSON *const object,
                                         const sbJSON *const target);

/* Sorts the members of the object into alphabetical order. */
void sbJSONUtils_SortObject(sbJSON *const object);

#ifdef __cplusplus
}
#endif

#endif
