/**
 * jsgen - JSON serialization/deserialization code generator for C
 * Example usage:
```c
#include "jsgen.h"

JSON struct role {
    int id;
    char *name;
    float *values sized_by("value_count");
    size_t value_count;
};

JSON typedef struct {
    int id;
    char *name;
    struct role *role;
    size_t role_count;
    bool is_active alias("active");
} User;

#include "models.g.h" // include the generated code
...
    User user = {0};
    Allocator a = {0}; // the parser will use this allocator for allocations
    parse_User("{...}", &user, &a);
    char *json_str = stringify_User(&user);
    printf("User as JSON: %s\n", json_str);
    jsgen_free(&a);
    free(json_str);
```
 */
#ifndef JSGEN_H
#define JSGEN_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#ifndef JSGEN_MALLOC
#define JSGEN_MALLOC malloc
#endif
#ifndef JSGEN_FREE
#define JSGEN_FREE free
#endif

// Generate JSON serialization/deserialization code for C struct.
#define JSGEN_JSON
// Generate only JSON stringification code for C struct.
#define JSGEN_JSONS
// Generate only JSON parsing code for C struct.
#define JSGEN_JSONP
// Ignore this field in JSON serialization/deserialization.
#define jsgen_ignore()
// Transform the field name in JSON to the indicated alias.
#define jsgen_alias(attr_alias)
// Define an array field that is sized by indicated field.
#define jsgen_sized_by(counter_field)

typedef struct JsGenRegion {
    size_t count;
    size_t capacity;
    struct JsGenRegion *next;
    uintptr_t items[];
} JsGenRegion;

#define ALL_REGION_MIN_SIZE (4096 - sizeof(JsGenRegion))
typedef struct {
    JsGenRegion *start, *end;
} JsGenAllocator;

void *jsgen_malloc(JsGenAllocator *a, size_t size);
void jsgen_free(JsGenAllocator *a);

void *jsgen_malloc(JsGenAllocator *a, size_t size) {
    if (size == 0) return NULL;
    size = (size + sizeof(uintptr_t) - 1) & ~(sizeof(uintptr_t) - 1);
    if (!a->end || a->end->count + size > a->end->capacity) {
        size_t region_size = sizeof(JsGenRegion) + (size > ALL_REGION_MIN_SIZE ? size : ALL_REGION_MIN_SIZE);
        JsGenRegion *r = JSGEN_MALLOC(region_size);
        if (!r) return NULL;
        r->count = sizeof(JsGenRegion);
        r->capacity = region_size;
        r->next = NULL;
        if (a->end) {
            a->end->next = r;
            a->end = r;
        } else {
            a->start = a->end = r;
        }
    }
    void *ptr = (void *)((uintptr_t)a->end->items + a->end->count);
    a->end->count += size;
    memset(ptr, 0, size);
    return ptr;
}

void jsgen_free(JsGenAllocator *a) {
    JsGenRegion *r = a->start;
    while (r) {
        JsGenRegion *next = r->next;
        JSGEN_FREE(r);
        r = next;
    }
    a->start = a->end = NULL;
}

#ifndef JSGEN_NO_STRIP
// Generate JSON serialization/deserialization code for C struct. Alias for JSGEN_JSON
#define JSON JSGEN_JSON
// Generate only JSON stringification code for C struct. Alias for JSGEN_JSONS
#define JSONS JSGEN_JSONS
// Generate only JSON parsing code for C struct. Alias for JSGEN_JSONP
#define JSONP JSGEN_JSONP
// Define an array field that is sized by indicated field. Alias for jsgen_sized_by
#define sized_by jsgen_sized_by
// Transform the field name in JSON to the indicated alias. Alias for jsgen_alias
#define alias jsgen_alias
// Region allocator for JSON parsing/stringifying. Alias for JsGenAllocator
#define Allocator JsGenAllocator
#endif // JSGEN_NO_STRIP

#endif // JSGEN_H
