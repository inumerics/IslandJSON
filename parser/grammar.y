/**
 * Parser specification for JSON.
 *
 * Defines the grammar and semantic actions that translate JSON text into an
 * in-memory representation. The rules cover all core JSON constructs: objects,
 * arrays, strings, numbers, booleans, and null values.
 *
 * Each production builds part of the syntax tree by invoking helper functions
 * from the JSON library to allocate and link values together. The completed
 * parse yields a structured `struct json` hierarchy suitable for traversal.
 */

%{

#include <stdint.h>

#include "json.h"


#include <string.h>
#include <stdbool.h>

#include "grammar.tab.h"

int yylex(void);
struct json *json_root = NULL;
void yyerror(const char *s);

%}

%token <string>  STRING
%token <number>  NUMBER
%token <boolean> BOOLEAN
%token NONE
%token ERROR_TOKEN

%union {
    struct json *json;
    uint8_t *string;
    double number;
    int boolean;
}

%type <json> object members
%type <json> array values value
%type <json> json

%%

json: value {
        $$ = $1;
        json_root = $$;
    };
value
    : object    { $$ = $1; }
    | array     { $$ = $1; }
    | STRING    { $$ = json_new_string($1);     }
    | NUMBER    { $$ = json_new_number($1);     }
    | BOOLEAN   { $$ = json_new_boolean($1);    }
    | NONE      { $$ = json_new_null();         }
    ;
object
    : '{' '}'  {
        $$ = json_new_object();
    }
    | '{' members '}' {
        $$ = $2;
    };
members
    : STRING ':' value {
        $$ = json_new_object();
        json_object_add($$, $1, $3);
    }
    | members ',' STRING ':' value {
        $$ = $1;
        json_object_add($$, $3, $5);
    };
array
    : '[' ']' {
        $$ = json_new_array();
    }
    | '[' values ']' {
        $$ = $2;
    };
values
    : value {
        $$ = json_new_array();
        json_array_add($$, $1);
    }
    | values ',' value {
        $$ = $1;
        json_array_add($$, $3);
    };

%%
    
