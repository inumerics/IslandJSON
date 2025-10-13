/**
 * Example program for testing the JSON library.
 *
 * Demonstrates how to include and link against the JSON library. The header
 * file "json.h" provides the public API declarations for parsing, printing,
 * and managing JSON values.
 *
 * The program reads JSON text from a file specified on the command line, or
 * from standard input if no filename is given. It parses the input into an
 * in-memory JSON structure and prints the result to standard output. Any
 * syntax errors are reported with a status code from the parser.
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
            
    if (json && status == JSON_SUCCESS) {
        printf("Parsing completed\n");
    } else {
        printf("Parsing failed with error code %d\n", status);
    }
    
    fclose(in);
    return (status != JSON_SUCCESS);
}
