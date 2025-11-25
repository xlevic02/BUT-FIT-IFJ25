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
    TT_BOOL,            // boolean = 9

    // Keywords defined by language
    TT_KEYWORD_CLASS,   // class = 10
    TT_KEYWORD_IF,      // if = 11
    TT_KEYWORD_ELSE,    // else = 12
    TT_KEYWORD_IS,      // is = 13
    TT_KEYWORD_RETURN,  // return = 14
    TT_KEYWORD_VAR,     // var = 15
    TT_KEYWORD_WHILE,   // while = 16
    TT_KEYWORD_IFJ,     // Ifj = 17
    TT_KEYWORD_STATIC,  // static = 18
    TT_KEYWORD_IMPORT,  // import = 19
    TT_KEYWORD_FOR,     // for = 20
    TT_KEYWORD_NUM,     // Num = 21
    TT_KEYWORD_STRING,  // String = 22
    TT_KEYWORD_Null,    // Null = 23

    // Operators
    TT_ASSIGN,          // = = 24
    TT_PLUS,            // + = 25
    TT_MINUS,           // - = 26
    TT_MUL,             // * = 27
    TT_DIV,             // / = 28
    TT_EQ,              // == = 29
    TT_NEQ,             // != = 30
    TT_LT,              // < = 31
    TT_GT,              // > = 32
    TT_LE,              // <= = 33
    TT_GE,              // >= = 34

    // Symbols
    TT_LPAREN,          // ( = 35
    TT_RPAREN,          // ) = 36
    TT_LBRACE,          // { = 37
    TT_RBRACE,          // } = 38
    TT_COMMA,           // , = 39
    TT_DOT,             // . = 40

    TT_ERROR            // ERROR = 41
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