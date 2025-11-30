// Implementace prekladace imperativniho jazyka IFJ25
// scanner.h by William Denis "xtihelw00" Tihelka on 10/13/25.
//

#ifndef SCANNER_H
#define SCANNER_H

#define INITIAL_BUF_CAP 16

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "error.h"

typedef enum {
    TT_EOF,             // end of file = 0
    TT_EOL,             // end of line = 1

    TT_IDENTIFIER,      // identifier = 2
    TT_NULL,            // null literal = 3
    TT_INT,             // integer = 4
    TT_FLOAT,           // float = 5
    TT_RUNNING_NUM,     // running number = 6
    TT_STRING,          // string = 7
    TT_RUNNING_STRING,  // running string = 8
    TT_RUNNING_UNDEF,   // running undefined = 9
    TT_BOOL,            // boolean = 10
    TT_TRUE,            // true = 11
    TT_FALSE,           // false = 12

    // Keywords defined by language
    TT_KEYWORD_CLASS,   // class = 13
    TT_KEYWORD_IF,      // if = 14
    TT_KEYWORD_ELSE,    // else = 15
    TT_KEYWORD_IS,      // is = 16
    TT_KEYWORD_RETURN,  // return = 17
    TT_KEYWORD_VAR,     // var = 18
    TT_KEYWORD_WHILE,   // while = 19
    TT_KEYWORD_IFJ,     // Ifj = 20
    TT_KEYWORD_STATIC,  // static = 21
    TT_KEYWORD_IMPORT,  // import = 22
    TT_KEYWORD_FOR,     // for = 23
    TT_KEYWORD_NUM,     // Num = 24
    TT_KEYWORD_STRING,  // String = 25
    TT_KEYWORD_Null,    // Null = 26

    // Operators
    TT_ASSIGN,          // = = 27
    TT_PLUS,            // + = 28
    TT_MINUS,           // - = 29
    TT_MUL,             // * = 30
    TT_DIV,             // / = 31
    TT_EQ,              // == = 32
    TT_NEQ,             // != = 33
    TT_LT,              // < = 34
    TT_GT,              // > = 35
    TT_LE,              // <= = 36
    TT_GE,              // >= = 37

    // Symbols
    TT_LPAREN,          // ( = 38
    TT_RPAREN,          // ) = 39
    TT_LBRACE,          // { = 40
    TT_RBRACE,          // } = 41
    TT_COMMA,           // , = 42
    TT_DOT,             // . = 43

    TT_ERROR            // ERROR = 44
} token_type_t;

typedef struct {
    token_type_t type;  // token type
    char *lexeme;       // orginial value (example: "while", "123", "x")
} token_t;

token_t get_token(void);

token_t make_token(token_type_t type, const char *lexeme);

token_type_t keyword_type(const char *lexeme);

void buf_append(char **buf, size_t *len, size_t *cap, char c);

#endif // SCANNER_H