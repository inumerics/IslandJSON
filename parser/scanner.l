/**
 * Lexical analyzer for JSON input.
 *
 * Recognizes structural tokens, literals, numbers, and strings according
 * to the JSON grammar. Whitespace and comments are ignored, while escape
 * sequences and Unicode code points are processed within quoted strings.
 * The tokens produced here form the input stream for the parser.
 */

%{

#ifndef FLEX_ONLY
    #include "json.h"
    #include "internal.h"
    #include "grammar.tab.h"
#else
    enum {
        BOOLEAN = 258,
        NUMBER,
        STRING,
        NONE,
        ERROR_TOKEN,
    };
#endif

%}

/* JSON number (ECMA-404) */
INT         (0|[1-9][0-9]*)
DECIMAL     (\.[0-9]+)
EXPONENT    ([eE][+-]?[0-9]+)
FLOATING    -?{INT}{DECIMAL}?{EXPONENT}?

/*CHAR     ([\u20-\u21]|[\u23-\u5B]|[\u5D-\u10FFFF])*/
CHAR     ([\x20-\x21]|[\x23-\x5B]|[\x5D-\x7E])
ESCAPE   (\\["\\/bfnrt])
HEX      ([0-9]|[a-f]|[A-F])
UNICODE  (\\u{HEX}{HEX}{HEX}{HEX})

%%

[ \t\n\r] { }
    
true    { return BOOLEAN; }
false   { return BOOLEAN; }
null    { return NONE;    }

"["     { return '['; }
"]"     { return ']'; }
"{"     { return '{'; }
"}"     { return '}'; }
":"     { return ':'; }
","     { return ','; }

{FLOATING} {
    #ifdef FLEX_ONLY
        printf("FLOATING: %s\n", yytext);
    #else
        yylval.number = scan_json_number(yytext);
        return NUMBER;
    #endif
}

\"({CHAR}|{ESCAPE}|{UNICODE})*\" {
    #ifdef FLEX_ONLY
        //printf("STRING: %s\n", yytext);
    #else
        yylval.string = scan_json_string(yytext);
    #endif
    return STRING;
}

. {
    #ifdef FLEX_ONLY
        printf("ERROR %s\n", yytext);
    #else
        return ERROR_TOKEN;
    #endif
}

%%

int yywrap(void) { return 1; }

