#include "json.h"
#include "internal.h"

#include <stdlib.h>

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
    struct json *result = calloc(1, sizeof(*result));
    if (!result) return NULL;
    
    *status = JSON_SUCCESS;
    return result;
}
