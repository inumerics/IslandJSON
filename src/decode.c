/**
 * Decodes the next UTF-8 code point from a buffer and advances the input pointer.
 *
 * This function reads a single well-formed UTF-8 sequence starting at `*ptr`
 * and writes the resulting Unicode scalar value to `out`. On success, the
 * pointer is advanced past the consumed bytes; on failure, the pointer is left
 * unchanged and false is returned.
 *
 * Validation notes:
 * - Rejects overlong encodings.
 * - Rejects UTF-16 surrogate code points (U+D800..U+DFFF).
 * - Enforces the Unicode maximum (U+10FFFF).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool
decode_2byte_UTF8(const uint8_t **ptr, const uint8_t *end, uint32_t *out)
{
    const uint8_t *p = *ptr;
    if (p >= end) return false;

    uint8_t c0 = p[0];
    uint32_t code;
    
    if (end - p < 2)
        return false;
    uint8_t c1 = p[1];
    
    if ((c1 & 0xC0) != 0x80)
        return false;
    
    code = ((c0 & 0x1F) << 6) | (c1 & 0x3F);
    
    if (code < 0x80) return false; // overlong
    p += 2;

    *ptr = p;
    *out = code;
    return true;
}

bool
decode_3byte_UTF8(const uint8_t **ptr, const uint8_t *end, uint32_t *out)
{
    const uint8_t *p = *ptr;
    if (p >= end) return false;

    uint8_t c0 = p[0];
    uint32_t code;
    
    if (end - p < 3)
        return false;
    uint8_t c1 = p[1], c2 = p[2];
    
    if ((c1 & 0xC0) != 0x80
     || (c2 & 0xC0) != 0x80)
        return false;
    
    code = ((c0 & 0x0F) << 12)
         | ((c1 & 0x3F) <<  6)
         |  (c2 & 0x3F);

    if (code <  0x0800) return false; // overlong
    if (code >= 0xD800 && code <= 0xDFFF) return false; // surrogate
    p += 3;

    *ptr = p;
    *out = code;
    return true;
}

bool
decode_4byte_UTF8(const uint8_t **ptr, const uint8_t *end, uint32_t *out)
{
    const uint8_t *p = *ptr;
    if (p >= end) return false;

    uint8_t c0 = p[0];
    uint32_t code;

    if (end - p < 4)
        return false;
    uint8_t c1 = p[1], c2 = p[2], c3 = p[3];
    
    if ((c1 & 0xC0) != 0x80
     || (c2 & 0xC0) != 0x80
     || (c3 & 0xC0) != 0x80)
        return false;
    
    code = ((c0 & 0x07) << 18)
         | ((c1 & 0x3F) << 12)
         | ((c2 & 0x3F) <<  6)
         |  (c3 & 0x3F);
    
    if (code < 0x010000) return false; /// overlong
    if (code > 0x10FFFF) return false; /// max range
    p += 4;
    
    *ptr = p;
    *out = code;
    return true;
}

bool
decode_next_UTF8(const uint8_t **ptr, const uint8_t *end, uint32_t *out)
{
    const uint8_t *p = *ptr;
    if (p >= end)
        return false;

    uint8_t c0 = *p;

    if (c0 < 0x80) {
        *out = c0;
        *ptr += 1;
        return true;
    }
    else if ((c0 & 0xE0) == 0xC0) {
        return decode_2byte_UTF8(ptr, end, out);
    }
    else if ((c0 & 0xF0) == 0xE0) {
        return decode_3byte_UTF8(ptr, end, out);
    }
    else if ((c0 & 0xF8) == 0xF0) {
        return decode_4byte_UTF8(ptr, end, out);
    }
    else {
        return false;
    }
}

