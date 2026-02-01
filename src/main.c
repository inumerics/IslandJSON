/**
 * JSON parsing and printing example.
 *
 * This program reads a well-formed JSON document from standard input or from
 * a file specified on the command line, parses it into an in-memory JSON
 * document, and writes the document back to standard output.
 *
 * Parsing is performed by a parser generated from a grammar based on the JSON
 * specification, with a dedicated lexer responsible for tokenizing the input
 * stream. This example demonstrates how lexer and parser generators can be
 * combined to build a robust, standards-compliant JSON processor.
 */

#include <stdlib.h>
#include <stdio.h>

#include "json.h"

int
main(int argc, const char * argv[])
{
    FILE *in = NULL;
    if (argc >= 2) {
        const char *filename = argv[1];
        in = fopen(filename, "r");
        if (!in) {
            fprintf(stderr, "Unable to open file.\n");
            return 1;
        }
    } else {
        in = stdin;
    }
    
    enum json_status status;
    struct json *json = json_parse(in, &status);
            
    if (status == JSON_SUCCESS) {
        json_print(json, stdout);
        fprintf(stdout, "\n\n");
    } else {
        printf("Parsing failed with error code %d\n", status);
    }
    
    fclose(in);
    return (status != JSON_SUCCESS);
}
