/**
 * Decodes a JSON-escaped string into a UTF-8 string.
 *
 * This function processes a JSON string containing escape sequences and returns
 * a dynamically allocated UTF-8 string with the escapes properly converted. It
 * supports standard JSON escape sequences and Unicode code points encoded as
 * \uXXXX. If an invalid escape sequence is encountered, the function returns
 * NULL and sets the error value.
 */

#include "internal.h"
#include "ustring.h"

/**
 * Converts a hex character ('0'-'F') to its integer value.
 */
static int
read_hex(uint8_t c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/**
 * Reads a 4-digit UTF-16 hexadecimal code from the input.  Advances the source
 * and returns the decoded value or -1 on error.
 */
static int
read_u16(const uint8_t **src, const uint8_t *end)
{
    if (*src + 4 > end)
        return -1;
    
    int code = 0;
    for (int i = 0; i < 4; i++) {
        int digit = read_hex(*(*src)++);
        if (digit < 0)
            return -1;
        
        code = (code << 4) | digit;
    }
    return code;
}

/**
 * Parses a Unicode escape, `\uXXXX`, handling surrogate pairs if needed and
 * returns the Unicode code point or -1 on error.
 */
static int
read_utf16(const uint8_t **src, const uint8_t *end)
{
    int code = read_u16(src, end);
    if (code < 0) return -1;
    
    if (code < 0xD800) {
        return code;
    }
    else if (code <= 0xDBFF) {
        if (*src + 2 > end) return -1;
        if (*(*src)++ != '\\') return -1;
        if (*(*src)++ !=  'u') return -1;
            
        int pair = read_u16(src, end);
        if (pair < 0) return -1;
        if (pair < 0xDC00 || pair > 0xDFFF) return -1;

        return 0x10000 + ((code - 0xD800) << 10) + (pair - 0xDC00);
    }
    else {
        return -1;
    }
}

/**
 * Converts escape sequences in a JSON string to their UTF8 character values.
 *
 * Returns a newly allocated unescaped string or NULL on error.  The caller is
 * responsible for freeing the returned string.
 */
uint8_t *
json_unescape_string(const uint8_t *start, size_t length, enum json_status *error)
{
    struct ustring *result = ustring_new(length);
    if (!result) return NULL;

    const uint8_t *src = start;
    const uint8_t *end = start + length;

    while (src < end) {
        int code = 0;
        
        if (*src == '\\' && (src + 1 < end)) {
            src++;
            char c = *src++;
            if (c == 'u') {
                code = read_utf16(&src, end);
                if (code < 0) {
                    if (error) *error = JSON_INVALID_UNICODE;
                    ustring_free(result);
                    return NULL;
                }
            } else {
                switch (c) {
                    case '"':  code = '"';  break;
                    case '\\': code = '\\'; break;
                    case '/':  code = '/';  break;
                    case 'b':  code = '\b'; break;
                    case 'f':  code = '\f'; break;
                    case 'n':  code = '\n'; break;
                    case 'r':  code = '\r'; break;
                    case 't':  code = '\t'; break;
                    default:  {
                        if (error) *error = JSON_INVALID_ESCAPE;
                        ustring_free(result);
                        return NULL;
                    }
                }
            }
        } else {
            code = *src++;
        }
                
        if (!ustring_push(result, code)) {
            ustring_free(result);
            return NULL;
        }
    }
    
    uint8_t *string = ustring_take_string(result);
    ustring_free(result);
    
    if (error) *error = JSON_SUCCESS;
    return string;
}

