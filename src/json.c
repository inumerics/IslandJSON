/**
 * JSON parsing, creation, and manipulation functions.
 *
 * This file provides the core functionality for parsing JSON input, managing
 * objects and arrays, and dynamically creating JSON values. The parsing system
 * is implemented using a lexer and a parser generator to read structured text.
 */

#include "json.h"
#include "internal.h"
#include "ustring.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

int yyparse(void);

extern FILE *yyin;
extern int yy_flex_debug;
extern struct json *json_root;

/**
 * Parses JSON text from the given input stream.
 *
 * Invokes the lexer and parser to build an in-memory representation of the
 * JSON data. On success, returns the root value; on failure, returns NULL
 * and sets an appropriate error code.
 */

struct json *
json_parse(FILE *in, enum json_status *status)
{
    yyin = in;
    yy_flex_debug = 0;
    
    int result = yyparse();
    if (result == 0) {
        *status = JSON_SUCCESS;
        return json_root;
    } else {
        *status = JSON_UNEXPECTED_CHARACTER;
        json_free(json_root);
        return NULL;
    }
}

/**
 * Functions for creating JSON values.
 *
 * Each function returns a heap-allocated JSON value of the requested type, or
 * NULL on allocation failure.
 */

struct json *
json_new_object(void)
{
    struct json *result = calloc(1, sizeof(*result));
    if (!result) return NULL;

    result->type = JSON_TYPE_OBJECT;
    json_object_init(&result->data.object);
    return result;
}

struct json *
json_new_array(void)
{
    struct json *result = calloc(1, sizeof(*result));
    if (!result) return NULL;

    result->type = JSON_TYPE_ARRAY;
    json_array_init(&result->data.array);
    return result;
}

struct json *
json_new_string(const uint8_t *string)
{
    struct json *result = calloc(1, sizeof(*result));
    if (!result) return NULL;

    result->type = JSON_TYPE_STRING;
    result->data.string = (uint8_t*)strdup((const char*)string);
    if (!result->data.string) {
        free(result);
        return NULL;
    }
    return result;
}

struct json *
json_new_number(double number)
{
    struct json *result = calloc(1, sizeof(*result));
    if (!result) return NULL;

    result->type = JSON_TYPE_NUMBER;
    result->data.number = number;
    return result;
}

struct json *
json_new_boolean(bool value)
{
    struct json *result = calloc(1, sizeof(*result));
    if (!result) return NULL;

    result->type = JSON_TYPE_BOOLEAN;
    result->data.boolean = value;
    return result;
}

struct json *
json_new_null(void)
{
    struct json *result = calloc(1, sizeof(*result));
    if (!result) return NULL;

    result->type = JSON_TYPE_NULL;
    return result;
}

void
json_free(struct json *value)
{
    if (!value) return;

    switch (value->type) {
        case JSON_TYPE_OBJECT:   json_object_release(&value->data.object); break;
        case JSON_TYPE_ARRAY:    json_array_release(&value->data.array); break;
        case JSON_TYPE_STRING:   free(value->data.string); break;
        case JSON_TYPE_NUMBER:   break;
        case JSON_TYPE_BOOLEAN:  break;
        case JSON_TYPE_NULL:     break;
    }
    free(value);
}

/**
 * Implementation of JSON objects.
 *
 * Each object maintains its key–value pairs in a linked list, allowing dynamic
 * insertion, removal, and iteration as values are parsed or constructed.
 */

void
json_object_init(struct json_object *object)
{
    if (!object) return;
    object->members = NULL;
}

void
json_object_release(struct json_object *object)
{
    if (!object) return;

    struct json_member *link = object->members;
    while (link) {
        struct json_member *next = link->next;
        json_member_free(link);
        link = next;
    }

    object->members = NULL;
}

struct json_member *
json_member_new(const uint8_t *key, struct json *value)
{
    struct json_member *result = malloc(sizeof(*result));
    if (!result) return NULL;

    result->key = (uint8_t*)strdup((char *)key);
    result->value = value;
    result->next = NULL;
    
    if (!result->key) {
        free(result);
        return NULL;
    }
    return result;
}

void
json_member_free(struct json_member *member)
{
    if (!member) return;
    json_free(member->value);
    free(member->key);
    free(member);
}

bool
json_object_add(struct json *json, const uint8_t *key, struct json *value)
{
    if (json->type != JSON_TYPE_OBJECT)
        return false;

    struct json_object *object = &json->data.object;
    struct json_member **link = &object->members;

    while (*link) {
        struct json_member *member = *link;

        if (strcmp((char *)member->key, (char *)key) == 0) {
            json_free(member->value);
            member->value = value;
            return true;
        }
        link = &member->next;
    }

    struct json_member *added = json_member_new(key, value);
    if (!added) return false;
    
    *link = added;
    return true;
}

struct json *
json_get_object_item(struct json *json, const uint8_t *key)
{
    if (json->type != JSON_TYPE_OBJECT)
        return NULL;

    struct json_member *link = json->data.object.members;
    while (link) {
        if (strcmp((char*)link->key, (char*)key) == 0) {
            return link->value;
        }
        link = link->next;
    }
    return NULL;
}

/**
 * Implementation of JSON arrays.
 *
 * Each array maintains an ordered sequence of values, allowing dynamic
 * insertion, removal, and iteration as elements are parsed or constructed.
 */

void
json_array_init(struct json_array *array)
{
    if (!array) return;
    array->items = NULL;
    array->capacity = 0;
    array->count = 0;
}

void
json_array_release(struct json_array *array)
{
    if (!array) return;
    
    for (size_t i = 0; i < array->count; i++) {
        json_free(array->items[i]);
    }
    
    free(array->items);
    array->items = NULL;
    array->capacity = 0;
    array->count = 0;
}

static bool
array_reserve(struct json_array *list, size_t n)
{
    const size_t max = SIZE_MAX / sizeof(*list->items);
    if (n <= list->capacity) return true;
    if (n > max) return false;

    size_t request = list->capacity ? list->capacity : 8;
    while (request < n)
        request = (request > max / 2) ? max : request * 2;

    void *resized = realloc(list->items, request * sizeof(*list->items));
    if (!resized) return false;

    list->items = resized;
    list->capacity = request;
    return true;
}

bool
json_array_add(struct json *value, struct json *item)
{
    if (value->type != JSON_TYPE_ARRAY)
        return false;
    
    struct json_array *array = &value->data.array;
    
    if (array->count == SIZE_MAX) return false;
    if (!array_reserve(array, array->count + 1)) return false;
    
    array->items[array->count++] = item;
    return true;
}

struct json *
json_get_array_item(struct json *json, size_t index)
{
    if (json->type != JSON_TYPE_ARRAY)
        return NULL;

    struct json_array *array = &json->data.array;
    return (index < array->count) ? array->items[index] : NULL;
}

/**
 * Functions used by the lexer while scanning tokens from the input stream.
 */

void yyerror(const char *s) {
    fprintf(stderr, "Lexical error: %s\n", s);
}

double
scan_json_number(const char *text)
{
    errno = 0;
    char *end = NULL;
    double value = strtod(text, &end);
    
    if (text == end) {
        yyerror("Unable to read number.\n");
        return 0;
    }
    else if (errno == ERANGE) {
        if (value == HUGE_VAL || value == -HUGE_VAL) {
            yyerror("Overflow occurred.\n");
        } else {
            yyerror("Underflow occurred.\n");
        }
        return 0;
    }
    else {
        return value;
    }
}

uint8_t *
scan_json_string(const char *text)
{
    size_t len = strlen(text);
    if (len < 2) {
        yyerror("Unable to read string.\n");
        return 0;
    }
    
    uint8_t *string = json_unescape_string((uint8_t*)text + 1, len - 2, NULL);
    if (!string) {
        yyerror("Unable to copy string.\n");
        return 0;
    }

    return string;
}
