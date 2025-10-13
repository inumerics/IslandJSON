/**
 * Functions for dynamic Unicode string handling in UTF-8 format.
 *
 * A Unicode string functions for dynamic UTF-8 handling, supporting creation,
 * deallocation, and appending Unicode code points.
 */

#ifndef USTRING_H
#define USTRING_H

struct ustring *ustring_new(size_t capacity);
void ustring_free(struct ustring *string);

bool ustring_push(struct ustring *string, uint32_t code);
bool ustring_reserve(struct ustring *list, size_t n);

/**
 * Extracts the underlying UTF-8 bytes from the string.
 *
 * The caller is now responsible for freeing the returned array.
 */
uint8_t *ustring_take_string(struct ustring *string);

/**
 * Decodes the next UTF-8 code point from a buffer and advances the input pointer.
 *
 * This function reads a single well-formed UTF-8 sequence starting at `*ptr`
 * and writes the resulting Unicode scalar value to `out`. On success, the
 * pointer is advanced past the consumed bytes; on failure, the pointer is left
 * unchanged and false is returned.
 */
bool decode_next_UTF8(const uint8_t **ptr, const uint8_t *end, uint32_t *out);

/**
 * Decodes a JSON string with escape sequences into a UTF-8 string.
 *
 * Processes a JSON string containing escape sequences and returns a dynamically
 * allocated UTF-8 string with the escapes properly converted. Supports standard
 * JSON escape sequences and Unicode code points encoded as `\uXXXX`. The caller
 * is responsible for freeing the returned memory.
 */
uint8_t *json_unescape_string(const uint8_t *start, size_t length,
                              enum json_status *error);

#endif // !USTRING_H
