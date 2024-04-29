// WIP sketch of possible new API design
// Goal: easier to scan through and understand entire API and consequences of each function

#include <stddef.h>
#include <stdint.h>

enum SbJSONKind {
    sbj_invalid,
    sbj_bool,
    sbj_null,
    sbj_number,
    sbj_string,
    sbj_array,
    sbj_object,
    sbj_raw,
};

typedef struct SbJSON {
    struct SbJSON *next;
    struct SbJSON *prev;
    struct SbJSON *child;

    uint8_t type;
    bool is_reference;
    bool string_is_const;
    bool is_number_double;
    int32_t child_count;

    union U {
        char *valuestring;
        int64_t valueint;
        double valuedouble;
        bool valuebool;
    } u;

    char *string;
} SbJSON;

typedef void *(*MallocFn)(size_t sz);
typedef void (*FreeFn)(void *ptr);

void sbj_init_hooks(MallocFn malloc_func, FreeFn free_func);

SbJSON *sbj_parse(char const* value);
SbJSON *sbj_parse_with_length(char const *value, size_t buffer_length);
SbJSON *sbj_parse_with_opts();
SbJSON *sbj_parse_with_length_opts();

SbJSON *sbj_print(SbJSON const *item);
SbJSON *sbj_print_unformatted(SbJSON const *item);
SbJSON *sbj_print_buffered(SbJSON const *item);
SbJSON *sbj_print_preallocated(SbJSON const *item);

SbJSON *sbj_copy(SbJSON const* item, bool recurse);
bool sbj_compare(SbJSON const* a, SbJSON const* b);

void sbj_minify(char *json);

SbJSON *sbj_delete(SbJSON const *item);

SbJSON *sbj_create_null(char const* name);
SbJSON *sbj_create_bool(char const* name, bool boolean);
SbJSON *sbj_create_double_number(char const* name, double number);
SbJSON *sbj_create_integer_number(char const* name, int64_t number);
SbJSON *sbj_create_string(char const* name, char const* string);
SbJSON *sbj_create_raw(char const* name, char const* string);
SbJSON *sbj_create_object(char const* name);
SbJSON *sbj_create_array(char const* name);

void sbj_attach_item(SbJSON *object, SbJSON *item);

// returns the new item, attached to the object
SbJSON *sbj_add_null(SbJSON *object, char const* name);
SbJSON *sbj_add_bool(SbJSON *object, char const* name, bool boolean);
SbJSON *sbj_add_double_number(SbJSON *object, char const* name, double number);
SbJSON *sbj_add_integer_number(SbJSON *object, char const* name, int64_t number);
SbJSON *sbj_add_string(SbJSON *object, char const* name, char const* string);
SbJSON *sbj_add_raw(SbJSON *object, char const* name, char const* string);
SbJSON *sbj_add_object(SbJSON *object, char const* name);
SbJSON *sbj_add_array(SbJSON *object, char const* name);

// remember to set value afterwards
void sbj_clear_to_kind(SbJSON *item, SbJSONKind kind);

// params called object or item?
void sbj_set_boolean(SbJSON *object, bool boolean);
void sbj_set_double_number(SbJSON *object, double number);
void sbj_set_integer_number(SbJSON *object, int64_t number);
void sbj_set_string(SbJSON *object, char const* string);
void sbj_set_raw(SbJSON *object, char const* string);

bool sbj_is_invalid(SbJSON const* item);
bool sbj_is_bool(SbJSON const* item);
bool sbj_is_null(SbJSON const* item);
bool sbj_is_number(SbJSON const* item);
bool sbj_is_string(SbJSON const* item);
bool sbj_is_array(SbJSON const* item);
bool sbj_is_object(SbJSON const* item);
bool sbj_is_raw(SbJSON const* item);

// for objects and arrays
int32_t sbj_item_count(SbJSON const* item);
void sbj_get_items(SbJSON const* item, SbJSON const** out_items);
void sbj_get_booleans(SbJSON const* array, bool** out_booleans);
void sbj_get_integers(SbJSON const* array, bool** out_booleans);
void sbj_get_doubles(SbJSON const* array, bool** out_booleans);

SbJSON *sbj_slow_get_array_item(SbJSON const *array, int index);

// null on failure
SbJSON *sbj_lookup(SbJSON const *object, char const* name);
SbJSON *obj_lookup_kind(SbJSON const *object, SbJSONKind kind, char const* name);

bool sbj_get_boolean(SbJSON *item);
int64_t sbj_get_integer(SbJSON *item);
double sbj_get_double(SbJSON *item);
char *sbj_get_string(SbJSON *item);

bool sbj_try_get_boolean(SbJSON *item, bool* boolean);
bool sbj_try_get_integer(SbJSON *item, int64_t* number);
bool sbj_try_get_double(SbJSON *item, double* number);
bool sbj_try_get_string(SbJSON *item, char* string);

bool sbj_get_boolean_or_default(SbJSON *item, bool default_bool);
double sbj_get_double_or_default(SbJSON *item, double default_number);
int64_t sbj_get_integer_or_default(SbJSON *item, int64_t default_number);
