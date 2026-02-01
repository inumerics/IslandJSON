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

#include <stdint.h>
#include "grammar.tab.h"

double   scan_json_number(const char *text);
uint8_t *scan_json_string(const char *text);

#else

enum {
    BOOLEAN = 0xE000,
    NUMBER,
    STRING,
    NONE,
    ERROR_TOKEN,
};

#endif

%}

%x COMMENT
%x SINGLE

/* JSON number (ECMA-404) */
INT         (0|[1-9][0-9]*)
DECIMAL     (\.[0-9]+)
EXPONENT    ([eE][+-]?[0-9]+)
FLOATING    -?{INT}{DECIMAL}?{EXPONENT}?

/* JSON5 number
 
SIGN        (+|-)
DIGIT       [0-9]
EXPONENT    ([eE][+-]?{DIGITS})
DIGITS      ({DIGIT}([_]?{DIGIT})*)
FLOATING    {SIGN}?{DIGITS}(\.{DIGITS}?)?{EXPONENT}?

BIN_DIGIT   [01]
HEX_DIGIT   [0-9A-Fa-f]
OCT_DIGIT   [0-7]
 
HEX_DIGITS     {HEX_DIGIT}(_?{HEX_DIGIT})*
BIN_DIGITS     [01](_?[01])*
OCT_DIGITS     {OCT_DIGIT}(_?{OCT_DIGIT})*
 
*/

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

"//"            { BEGIN(SINGLE); }
<SINGLE>"\n"    { BEGIN(INITIAL); }
<SINGLE>[^\n]   { }

"/*"            { BEGIN(COMMENT); }
<COMMENT>"*/"   { BEGIN(INITIAL); }
<COMMENT>.      { }

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

