/**
 * Internal structures and definitions for JSON parsing.
 *
 * Defines data structures for representing JSON objects, arrays, strings,
 * numbers, booleans, and null values.  These structures and functions are for
 * internal use only and should not be exposed in the public API. Users should
 * interact with JSON data through the public interface.
 */

#ifndef INTERNAL_H
#define INTERNAL_H

#include "json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**
 * JSON object containing key-value pairs.
 *
 * A JSON object stores an unordered collection of unique key-value pairs,
 * where each key is a UTF-8 encoded string and each value is a JSON data type.
 * Members are stored as a linked list for dynamic and flexible storage.
 */

struct json_object {
    struct json_member *members;
};

void json_object_init(struct json_object *object);
void json_object_release(struct json_object *object);

struct json_member {
    uint8_t *key;
    struct json *value;
    struct json_member *next;
};

struct json_member *json_member_new(const uint8_t *key, struct json *value);
void json_member_free(struct json_member *member);

/**
 * A JSON array is an ordered list of JSON values, dynamically allocated
 * with adjustable capacity to accommodate elements as needed.
 */

struct json_array {
    struct json **items;
    size_t capacity;
    size_t count;
};

void json_array_init(struct json_array *array);
void json_array_release(struct json_array *array);

/**
 * Generic JSON value.
 *
 * This structure stores a JSON value of any type, using a tagged union.
 * This structure serves as the core representation of JSON data, allowing
 * dynamic storage and manipulation of objects, arrays, strings, numbers,
 * booleans, and null values.
 */

struct json {
    enum json_type type;
    
    union {
        struct json_object object;
        struct json_array array;
        uint8_t *string;
        double number;
        bool boolean;
    } data;
};

/**
 * Parses a JSON number and returns its value as double.
 *
 * This function is used by the lexer while scanning numeric tokens from the
 * input stream. It converts the textual representation of a JSON number into
 * a double-precision value.
 */
double scan_json_number(const char *text);

/**
 * Decodes a JSON string literal into a newly allocated UTF-8 C string.
 *
 * This function is used by the lexer while scanning string tokens from the
 * input stream. It handles escape sequences and Unicode decoding as defined
 * by the JSON specification.
 */
uint8_t *scan_json_string(const char *text);

#endif
