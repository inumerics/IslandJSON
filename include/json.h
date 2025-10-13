/**
 * Lightweight JSON library for creating, parsing, modifying, and querying
 * JSON values.
 *
 *   - Ownership is explicit: callers own the values they create and must
 *     release them, while containers own and free their elements.
 *   - All text is UTF-8 encoded; string parameters are arrays of uint8_t.
 *   - Parsing functions return NULL on failure and set a status value
 *     describing the error.
 */

#ifndef JSON_H
#define JSON_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * Opaque handle to a JSON value.
 *
 * A forward-declared `struct json` represents any JSON value: object, array,
 * string, number, boolean, or null. Its internal layout is private to the
 * library; all access and modification should be done through the functions
 * declared in this header.
 */

struct json;

/**
 * Value creation functions.
 *
 * These functions allocate and initialize new JSON values of each type.
 * The returned value is owned by the caller and must be freed, unless it is
 * inserted into a container (object or array), which then assumes ownership.
 */

struct json *json_new_object(void);
struct json *json_new_array(void);
struct json *json_new_string(const uint8_t *string);
struct json *json_new_number(double number);
struct json *json_new_boolean(bool value);
struct json *json_new_null(void);

void json_free(struct json *value);

/**
 * Parsing and printing functions.
 *
 * Reads JSON text from a stream and constructs the corresponding value tree.
 * On success, it returns the root value and sets status to `JSON_SUCCESS`.
 * On failure, it returns NULL and sets status to describe the first error
 * encountered.
 */

enum json_status {
    JSON_SUCCESS,
    JSON_UNEXPECTED_CHARACTER,
    JSON_UNEXPECTED_FILE_END,
    JSON_INVALID_ESCAPE,
    JSON_INVALID_UNICODE
};

struct json *json_parse(FILE *in, enum json_status *status);
void json_print(const struct json *json, FILE *out);

/**
 * Data types.
 *
 * Every JSON value has one of these types. The enumeration identifies the kind
 * of value stored in a struct json, allowing code to inspect or validate data
 * before accessing it.
 */

enum json_type {
    JSON_TYPE_OBJECT,
    JSON_TYPE_ARRAY,
    JSON_TYPE_STRING,
    JSON_TYPE_NUMBER,
    JSON_TYPE_BOOLEAN,
    JSON_TYPE_NULL
};

enum json_type json_type(const struct json *json);

/**
 * Modify JSON objects by adding or removing key–value pairs.
 *
 * When adding a pair, any existing entry with the same key is replaced and the
 * old value is released. Ownership of the inserted value transfers to the
 * object on success.
 *
 * Removing a pair deletes the entry for the specified key. If the key does not
 * exist, the operation has no effect and returns false.
 */

bool json_object_add(struct json *json, const uint8_t *key, struct json *value);
bool json_object_remove(struct json *json, const uint8_t *key);

/**
 * Modify JSON arrays by adding, removing, and inspecting elements.
 *
 * When adding a value, ownership transfers to the array on success. Removing
 * an element releases its storage and shifts subsequent elements down by one
 * position.
 */

bool json_array_add(struct json *json, struct json *value);
bool json_array_remove(struct json *json, size_t index);
size_t json_array_length(const struct json *json);

/**
 * Read access to values stored within objects and arrays.
 *
 * When applied to an object, retrieves the value associated with a given key;
 * when applied to an array, accesses an element by index.
 */

struct json    *json_object_get(const struct json *json, const uint8_t *key);
struct json    *json_array_get(const struct json *json, size_t index);
const uint8_t  *json_get_string(const struct json *json);
double          json_get_number(const struct json *json);
bool            json_get_boolean(const struct json *json);

#endif // !JSON_H
