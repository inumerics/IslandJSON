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

#endif // !JSON_H
