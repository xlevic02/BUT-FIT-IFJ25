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
    TT_STRING,          // string = 6

    // Keywords defined by language
    TT_KEYWORD_CLASS,   // class = 7
    TT_KEYWORD_IF,      // if = 8
    TT_KEYWORD_ELSE,    // else = 9
    TT_KEYWORD_IS,      // is = 10
    TT_KEYWORD_RETURN,  // return = 11
    TT_KEYWORD_VAR,     // var = 12
    TT_KEYWORD_WHILE,   // while = 13
    TT_KEYWORD_IFJ,     // Ifj = 14
    TT_KEYWORD_STATIC,  // static = 15
    TT_KEYWORD_IMPORT,  // import = 16
    TT_KEYWORD_FOR,     // for = 17
    TT_KEYWORD_NUM,     // Num = 18
    TT_KEYWORD_STRING,  // String = 19
    TT_KEYWORD_Null,    // Null = 20

    // Operators
    TT_ASSIGN,          // = = 21
    TT_PLUS,            // + = 22
    TT_MINUS,           // - = 23
    TT_MUL,             // * = 24
    TT_DIV,             // / = 25
    TT_EQ,              // == = 26
    TT_NEQ,             // != = 27
    TT_LT,              // < = 28
    TT_GT,              // > = 29
    TT_LE,              // <= = 30
    TT_GE,              // >= = 31

    // Symbols
    TT_LPAREN,          // ( = 32
    TT_RPAREN,          // ) = 33
    TT_LBRACE,          // { = 34
    TT_RBRACE,          // } = 35
    TT_COMMA,           // , = 36
    TT_DOT,             // . = 37

    TT_ERROR            // ERROR = 38
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