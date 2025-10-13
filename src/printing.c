/**
 * JSON pretty-printing helpers.
 *
 * Provides functions for printing JSON objects, arrays, and values in a
 * human-readable, indented form. Supports multi-line formatting for nested
 * structures. Intended for internal use within the JSON library.
 */

#include "internal.h"
#include "ustring.h"

/**
 * If the object or array includes at least one object or another array, it
 * should be printed with indentation for better readability.
 */

static bool
object_contains_object_or_array(const struct json_object *object)
{
    struct json_member *member = object->members;
    while (member) {
        if (member->value->type == JSON_TYPE_OBJECT ||
            member->value->type == JSON_TYPE_ARRAY) {
            return true;
        }
        member = member->next;
    }
    return false;
}

static bool
array_contains_object_or_array(const struct json_array *array)
{
    for (size_t i = 0; i < array->count; i++) {
        if (array->items[i]->type == JSON_TYPE_OBJECT ||
            array->items[i]->type == JSON_TYPE_ARRAY) {
            return true;
        }
    }
    return false;
}

/**
 * Emit a Unicode code point as JSON \u escapes (ASCII-only output).
 *
 * Encodes the given Unicode scalar value using standard JSON escape sequences.
 * For characters beyond the Basic Multilingual Plane (U+10000 and above),
 * surrogate pairs are generated.
 */

static void
json_print_escaped(FILE *out, uint32_t code)
{
    if (code <= 0xFFFF) {
        fprintf(out, "\\u%04X", (uint16_t)code);
    } else {
        uint32_t v  = code - 0x10000;
        uint32_t hi = 0xD800 + (v >> 10);
        uint32_t lo = 0xDC00 + (v & 0x3FF);
        fprintf(out, "\\u%04X\\u%04X", (uint16_t)hi, (uint16_t)lo);
    }
}

static void
json_print_question_mark(FILE *out, bool ascii)
{
    if (ascii) {
        fputs("\\uFFFD", out);
    } else {
        fputc(0xEF, out);
        fputc(0xBF, out);
        fputc(0xBD, out);
    }
}

/**
 * Print a UTF-8 string as a JSON string.
 *
 * Writes a UTF-8 string to the output stream, enclosing it in double quotes
 * and escaping all control characters and special symbols as required by the
 * JSON specification.  If ascii is true, non-ASCII is emitted as \uXXXX
 * (with surrogate pairs).
 */

static void
json_print_string(const uint8_t *text, FILE *out, bool ascii)
{
    fputc('"', out);
    
    size_t length = strlen((const char*)text);
    const uint8_t *end = text + length;

    for (const uint8_t *p = text; *p; ) {
        uint32_t code;
        const uint8_t *start = p;
        
        if (!decode_next_UTF8(&p, end, &code)) {
            json_print_question_mark(out, ascii);
            
            p += 1;
            size_t skips = 0;
            while ((skips < 3) && (p < end)
                   && ((*p & 0xC0) == 0x80)) {
                p++; skips++;
            }
            continue;
        }

        switch (code) {
        case '\"': fputs("\\\"", out); break;
        case '\\': fputs("\\\\", out); break;
        case '\b': fputs("\\b",  out); break;
        case '\f': fputs("\\f",  out); break;
        case '\n': fputs("\\n",  out); break;
        case '\r': fputs("\\r",  out); break;
        case '\t': fputs("\\t",  out); break;

        default:
            if (code < 0x20) {
                json_print_escaped(out, code);
            }
            else if (ascii && code >= 0x80) {
                json_print_escaped(out, code);
            }
            else {
                const uint8_t *next = start;
                while (next < p) fputc(*next++, out);
            }
            break;
        }
    }

    fputc('"', out);
}

/**
 * Produces valid JSON text by recursively writing objects, arrays, and
 * primitive values in standard syntax.
 */

static void json_print_indent(const struct json *json, FILE *out, int indent);

static void
print_indent(FILE *out, int indent)
{
    for (int i = 0; i < indent; i++) {
        fputc(' ', out);
    }
}

static void
print_object(const struct json_object *object, FILE *out, int indent)
{
    bool multi = object_contains_object_or_array(object);

    fprintf(out, multi ? "{\n" : "{");

    struct json_member *member = object->members;
    for (; member; member = member->next)
    {
        if (multi) print_indent(out, indent + 2);
        json_print_string(member->key, out, false);
        fprintf(out, ": ");
        
        json_print_indent(member->value, out, indent + 2);

        if (member->next) fprintf(out, ", ");
        if (multi) fprintf(out, "\n");
    }

    if (multi) print_indent(out, indent);
    fprintf(out, "}");
}

static void
print_array(const struct json_array *array, FILE *out, int indent)
{
    bool multiline = array_contains_object_or_array(array);

    fprintf(out, multiline ? "[\n" : "[");

    for (size_t i = 0; i < array->count; i++) {
        if (multiline) print_indent(out, indent + 2);
        
        json_print_indent(array->items[i], out, indent + 2);

        if (i < array->count - 1) fprintf(out, ", ");
        if (multiline) fprintf(out, "\n");
    }

    if (multiline) print_indent(out, indent);
    fprintf(out, "]");
}

/**
 * Dispatches to the appropriate print function for objects, arrays, strings,
 * numbers, booleans, and null values.
 */

static void
json_print_indent(const struct json *json, FILE *out, int indent)
{
    if (!json) return;

    switch (json->type) {
        case JSON_TYPE_OBJECT:
            print_object(&json->data.object, out, indent);
            break;
        case JSON_TYPE_ARRAY:
            print_array(&json->data.array, out, indent);
            break;
        case JSON_TYPE_STRING:
            json_print_string(json->data.string, out, false);
            break;
        case JSON_TYPE_NUMBER:
            fprintf(out, "%f", json->data.number);
            break;
        case JSON_TYPE_BOOLEAN:
            fprintf(out, json->data.boolean ? "true" : "false");
            break;
        case JSON_TYPE_NULL:
            fprintf(out, "null");
            break;
    }
}

void
json_print(const struct json *json, FILE *out)
{
    int ident = 0;
    json_print_indent(json, out, ident);
    fprintf(out, "\n");
}
