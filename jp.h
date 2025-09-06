/**
 * Simple JSON parser
 * https://github.com/mceck/c-stb
 *
 * Example:
```c
#define JP_IMPLEMENTATION
#include "jp.h"
...
    JpNode *root = jp_parse(json);
    JpNode *name = jp_get(root, "name");
    printf("Name: %s\n", name->string_val);
```
 */

#ifndef JP_H_
#define JP_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

// JSON value types
typedef enum {
    JP_NULL,
    JP_BOOL,
    JP_NUMBER,
    JP_STRING,
    JP_ARRAY,
    JP_OBJECT
} JpType;

// Forward declaration
struct JpNode;

// Object key-value pair
typedef struct JpPair {
    char *key;
    struct JpNode *value;
    struct JpPair *next;
} JpPair;

// Array element
typedef struct JpArrayItem {
    struct JpNode *value;
    struct JpArrayItem *next;
} JpArrayItem;

// JSON node structure
typedef struct JpNode {
    JpType type;
    union {
        bool bool_val;
        double number_val;
        char *string_val;
        JpPair *object_val;
        JpArrayItem *array_val;
    };
} JpNode;

// Parser state
typedef struct {
    const char *text;
    size_t pos;
    size_t len;
} JpParser;

// Function declarations
JpNode *jp_parse(const char *text);
JpNode *jp_get(JpNode *node, const char *key);
JpNode *jp_array_get(JpNode *node, int index);
size_t jp_array_len(JpNode *node);
void jp_free(JpNode *node);
void jp_print(JpNode *node, int indent);

#ifdef JP_IMPLEMENTATION
// Helper functions
static void skip_whitespace(JpParser *parser);
static JpNode *parse_value(JpParser *parser);
static JpNode *parse_object(JpParser *parser);
static JpNode *parse_array(JpParser *parser);
static JpNode *parse_string(JpParser *parser);
static JpNode *parse_number(JpParser *parser);
static JpNode *parse_literal(JpParser *parser);
static char *extract_string(JpParser *parser);

// Create a new JSON node
static JpNode *JpNode_new(JpType type) {
    JpNode *node = malloc(sizeof(JpNode));
    if (!node) return NULL;
    node->type = type;
    memset(&node->string_val, 0, sizeof(node->string_val));
    return node;
}

// Skip whitespace characters
static void skip_whitespace(JpParser *parser) {
    while (parser->pos < parser->len && isspace(parser->text[parser->pos])) {
        parser->pos++;
    }
}

// Parse main entry point
JpNode *jp_parse(const char *text) {
    if (!text) return NULL;

    JpParser parser = {
        .text = text,
        .pos = 0,
        .len = strlen(text)};

    skip_whitespace(&parser);
    return parse_value(&parser);
}

// Parse any JSON value
static JpNode *parse_value(JpParser *parser) {
    skip_whitespace(parser);
    if (parser->pos >= parser->len) return NULL;

    char c = parser->text[parser->pos];

    if (c == '{') return parse_object(parser);
    if (c == '[') return parse_array(parser);
    if (c == '"') return parse_string(parser);
    if (c == '-' || isdigit(c)) return parse_number(parser);
    if (c == 't' || c == 'f' || c == 'n') return parse_literal(parser);

    return NULL;
}

// Parse JSON object
static JpNode *parse_object(JpParser *parser) {
    if (parser->text[parser->pos] != '{') return NULL;
    parser->pos++; // skip '{'

    JpNode *node = JpNode_new(JP_OBJECT);
    if (!node) return NULL;

    JpPair *first_pair = NULL;
    JpPair *last_pair = NULL;

    skip_whitespace(parser);

    // Empty object
    if (parser->pos < parser->len && parser->text[parser->pos] == '}') {
        parser->pos++;
        node->object_val = NULL;
        return node;
    }

    while (parser->pos < parser->len) {
        skip_whitespace(parser);

        // Parse key
        if (parser->text[parser->pos] != '"') break;
        char *key = extract_string(parser);
        if (!key) break;

        skip_whitespace(parser);
        if (parser->pos >= parser->len || parser->text[parser->pos] != ':') {
            free(key);
            break;
        }
        parser->pos++; // skip ':'

        // Parse value
        JpNode *value = parse_value(parser);
        if (!value) {
            free(key);
            break;
        }

        // Create pair
        JpPair *pair = malloc(sizeof(JpPair));
        if (!pair) {
            free(key);
            jp_free(value);
            break;
        }
        pair->key = key;
        pair->value = value;
        pair->next = NULL;

        if (!first_pair) {
            first_pair = pair;
            last_pair = pair;
        } else {
            last_pair->next = pair;
            last_pair = pair;
        }

        skip_whitespace(parser);
        if (parser->pos >= parser->len) break;

        if (parser->text[parser->pos] == '}') {
            parser->pos++;
            break;
        } else if (parser->text[parser->pos] == ',') {
            parser->pos++;
        } else {
            break;
        }
    }

    node->object_val = first_pair;
    return node;
}

// Parse JSON array
static JpNode *parse_array(JpParser *parser) {
    if (parser->text[parser->pos] != '[') return NULL;
    parser->pos++; // skip '['

    JpNode *node = JpNode_new(JP_ARRAY);
    if (!node) return NULL;

    JpArrayItem *first_item = NULL;
    JpArrayItem *last_item = NULL;

    skip_whitespace(parser);

    // Empty array
    if (parser->pos < parser->len && parser->text[parser->pos] == ']') {
        parser->pos++;
        node->array_val = NULL;
        return node;
    }

    while (parser->pos < parser->len) {
        JpNode *value = parse_value(parser);
        if (!value) break;

        JpArrayItem *item = malloc(sizeof(JpArrayItem));
        if (!item) {
            jp_free(value);
            break;
        }
        item->value = value;
        item->next = NULL;

        if (!first_item) {
            first_item = item;
            last_item = item;
        } else {
            last_item->next = item;
            last_item = item;
        }

        skip_whitespace(parser);
        if (parser->pos >= parser->len) break;

        if (parser->text[parser->pos] == ']') {
            parser->pos++;
            break;
        } else if (parser->text[parser->pos] == ',') {
            parser->pos++;
        } else {
            break;
        }
    }

    node->array_val = first_item;
    return node;
}

// Parse JSON string
static JpNode *parse_string(JpParser *parser) {
    char *str = extract_string(parser);
    if (!str) return NULL;

    JpNode *node = JpNode_new(JP_STRING);
    if (!node) {
        free(str);
        return NULL;
    }
    node->string_val = str;
    return node;
}

// Extract string value (helper)
static char *extract_string(JpParser *parser) {
    if (parser->text[parser->pos] != '"') return NULL;
    parser->pos++; // skip opening quote

    size_t start = parser->pos;
    size_t len = 0;
    while (parser->pos < parser->len && parser->text[parser->pos] != '"') {
        if (parser->text[parser->pos] == '\\') {
            parser->pos++;
        }
        parser->pos++;
        len++;
    }

    if (parser->pos >= parser->len) return NULL;
    char *str = malloc(len + 1);
    if (!str) return NULL;
    for (size_t i = 0, j = 0; j < len; i++, j++) {
        if (parser->text[start + i] == '\\') {
            char next = parser->text[start + i + 1];
            if (next == 'n') str[j] = '\n';
            if (next == 't') str[j] = '\t';
            if (next == '\\') str[j] = '\\';
            if (next == '"') str[j] = '"';
            i++;
            continue;
        }
        str[j] = parser->text[start + i];
    }
    str[len] = '\0';

    parser->pos++; // skip closing quote
    return str;
}

// Parse JSON number
static JpNode *parse_number(JpParser *parser) {
    size_t start = parser->pos;

    if (parser->text[parser->pos] == '-') parser->pos++;

    while (parser->pos < parser->len && isdigit(parser->text[parser->pos])) {
        parser->pos++;
    }

    if (parser->pos < parser->len && parser->text[parser->pos] == '.') {
        parser->pos++;
        while (parser->pos < parser->len && isdigit(parser->text[parser->pos])) {
            parser->pos++;
        }
    }

    if (parser->pos < parser->len && (parser->text[parser->pos] == 'e' || parser->text[parser->pos] == 'E')) {
        parser->pos++;
        if (parser->pos < parser->len && (parser->text[parser->pos] == '+' || parser->text[parser->pos] == '-')) {
            parser->pos++;
        }
        while (parser->pos < parser->len && isdigit(parser->text[parser->pos])) {
            parser->pos++;
        }
    }

    size_t len = parser->pos - start;
    char *num_str = malloc(len + 1);
    if (!num_str) return NULL;

    strncpy(num_str, parser->text + start, len);
    num_str[len] = '\0';

    JpNode *node = JpNode_new(JP_NUMBER);
    if (!node) {
        free(num_str);
        return NULL;
    }

    node->number_val = atof(num_str);
    free(num_str);
    return node;
}

// Parse JSON literals (true, false, null)
static JpNode *parse_literal(JpParser *parser) {
    if (parser->pos + 4 <= parser->len && strncmp(parser->text + parser->pos, "true", 4) == 0) {
        parser->pos += 4;
        JpNode *node = JpNode_new(JP_BOOL);
        if (node) node->bool_val = 1;
        return node;
    }

    if (parser->pos + 5 <= parser->len && strncmp(parser->text + parser->pos, "false", 5) == 0) {
        parser->pos += 5;
        JpNode *node = JpNode_new(JP_BOOL);
        if (node) node->bool_val = 0;
        return node;
    }

    if (parser->pos + 4 <= parser->len && strncmp(parser->text + parser->pos, "null", 4) == 0) {
        parser->pos += 4;
        return JpNode_new(JP_NULL);
    }

    return NULL;
}

// Get value from object by key
JpNode *jp_get(JpNode *node, const char *key) {
    if (!node || node->type != JP_OBJECT || !key) return NULL;

    JpPair *pair = node->object_val;
    while (pair) {
        if (strcmp(pair->key, key) == 0) {
            return pair->value;
        }
        pair = pair->next;
    }
    return NULL;
}

// Get value from array by index
JpNode *jp_array_get(JpNode *node, int index) {
    if (!node || node->type != JP_ARRAY || index < 0) return NULL;

    JpArrayItem *item = node->array_val;
    int i = 0;
    while (item && i < index) {
        item = item->next;
        i++;
    }

    return item ? item->value : NULL;
}

size_t jp_array_len(JpNode *node) {
    if (!node || node->type != JP_ARRAY) return 0;

    size_t size = 0;
    JpArrayItem *item = node->array_val;
    while (item) {
        size++;
        item = item->next;
    }
    return size;
}

// Free JSON node and all its children
void jp_free(JpNode *node) {
    if (!node) return;

    switch (node->type) {
    case JP_STRING:
        free(node->string_val);
        break;
    case JP_OBJECT: {
        JpPair *pair = node->object_val;
        while (pair) {
            JpPair *next = pair->next;
            free(pair->key);
            jp_free(pair->value);
            free(pair);
            pair = next;
        }
        break;
    }
    case JP_ARRAY: {
        JpArrayItem *item = node->array_val;
        while (item) {
            JpArrayItem *next = item->next;
            jp_free(item->value);
            free(item);
            item = next;
        }
        break;
    }
    default:
        break;
    }

    free(node);
}

// Print JSON for debugging
void jp_print(JpNode *node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++)
        printf("  ");

    switch (node->type) {
    case JP_NULL:
        printf("null\n");
        break;
    case JP_BOOL:
        printf("%s\n", node->bool_val ? "true" : "false");
        break;
    case JP_NUMBER:
        printf("%.6g\n", node->number_val);
        break;
    case JP_STRING:
        printf("\"%s\"\n", node->string_val);
        break;
    case JP_OBJECT:
        printf("{\n");
        JpPair *pair = node->object_val;
        while (pair) {
            for (int i = 0; i < indent + 1; i++)
                printf("  ");
            printf("\"%s\": ", pair->key);
            if (pair->value->type == JP_OBJECT || pair->value->type == JP_ARRAY) {
                printf("\n");
                jp_print(pair->value, indent + 1);
            } else {
                jp_print(pair->value, 0);
            }
            pair = pair->next;
        }
        for (int i = 0; i < indent; i++)
            printf("  ");
        printf("}\n");
        break;
    case JP_ARRAY:
        printf("[\n");
        JpArrayItem *item = node->array_val;
        while (item) {
            jp_print(item->value, indent + 1);
            item = item->next;
        }
        for (int i = 0; i < indent; i++)
            printf("  ");
        printf("]\n");
        break;
    }
}
#endif // JP_IMPLEMENTATION
#endif // JP_H_