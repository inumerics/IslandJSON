/**
 * Unicode string library for UTF-8 handling.
 *
 * This library provides a way to manage dynamically allocated UTF-8 strings,
 * supporting creation, resizing, and appending Unicode code points.
 */

#include "internal.h"
#include "ustring.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

struct ustring {
    uint8_t *data;
    size_t capacity;
    size_t length;
};

struct ustring *
ustring_new(size_t capacity)
{
    struct ustring *result = malloc(sizeof(*result));
    if (!result) return NULL;
    
    result->data = NULL;
    result->capacity = 0;
    result->length = 0;
    
    if (!ustring_reserve(result, capacity)) {
        free(result);
        return NULL;
    }
    
    return result;
}

void
ustring_free(struct ustring *ustr)
{
    if (!ustr) return;
    free(ustr->data);
    free(ustr);
}

bool
ustring_reserve(struct ustring *ustr, size_t n)
{
    const size_t max = SIZE_MAX / sizeof(*ustr->data);
    if (n <= ustr->capacity) return true;
    if (n > max) return false;

    size_t request = ustr->capacity ? ustr->capacity : 16;
    while (request < n)
        request = (request > max / 2) ? max : request * 2;

    void *resized = realloc(ustr->data, request * sizeof(*ustr->data));
    if (!resized) return false;

    ustr->data = resized;
    ustr->capacity = request;
    return true;
}

/**
 * Converts a given Unicode code point to UTF-8 and appends it to the string
 * buffer.  If necessary, resizes the buffer to accommodate the new character.
 */
bool
ustring_push(struct ustring *dst, uint32_t code)
{
    char bytes[4];
    size_t count = 0;

    if (code <= 0x7F) {
        count = 1;
        bytes[0] = (char)code;
    }
    else if (code <= 0x7FF) {
        count = 2;
        bytes[0] = (char)(0xC0 | ((code >> 6) & 0x1F));
        bytes[1] = (char)(0x80 | (code & 0x3F));
    }
    else if (code <= 0xFFFF) {
        count = 3;
        bytes[0] = (char)(0xE0 | ((code >> 12) & 0x0F));
        bytes[1] = (char)(0x80 | ((code >> 6) & 0x3F));
        bytes[2] = (char)(0x80 | (code & 0x3F));
    }
    else if (code <= 0x10FFFF) {
        count = 4;
        bytes[0] = (char)(0xF0 | ((code >> 18) & 0x07));
        bytes[1] = (char)(0x80 | ((code >> 12) & 0x3F));
        bytes[2] = (char)(0x80 | ((code >> 6) & 0x3F));
        bytes[3] = (char)(0x80 | (code & 0x3F));
    }
    else {
        return false;
    }

    if ((count + 1) > (SIZE_MAX - dst->length)) {
        return false;
    }
    if (!ustring_reserve(dst, dst->length + count + 1)) {
        return false;
    }

    for (size_t i = 0; i < count; i++) {
        dst->data[dst->length++] = bytes[i];
    }
    dst->data[dst->length] = '\0';
    
    return true;
}

/**
 * Returns the internal string buffer and resets the structure to be an empty
 * string. The caller is responsible for freeing the returned buffer.
 */
uint8_t *
ustring_take_string(struct ustring *ustr)
{
    if (!ustr || !ustr->data) return NULL;
    
    uint8_t *result = ustr->data;
    
    ustr->data = NULL;
    ustr->capacity = 0;
    ustr->length = 0;
    
    return result;
}
