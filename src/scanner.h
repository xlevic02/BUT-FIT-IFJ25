// Implementace prekladace imperativniho jazyka IFJ25
// scanner.h by William Denis "xtihelw00" Tihelka on MM/DD/25.
//

#ifndef SCANNER_H
#define SCANNER_H

typedef enum {
    TT_EOF,             // end of file
    TT_EOL,             // end of line

    TT_IDENTIFIER,      // identifier
    TT_NULL,            // null literal
    TT_INT,             // integer
    TT_FLOAT,           // float
    TT_STRING,          // string

    // Keywords defined by language
    TT_KEYWORD_CLASS,   // class
    TT_KEYWORD_IF,      // if
    TT_KEYWORD_ELSE,    // else
    TT_KEYWORD_IS,      // is
    TT_KEYWORD_RETURN,  // return
    TT_KEYWORD_VAR,     // var
    TT_KEYWORD_WHILE,   // while
    TT_KEYWORD_IFJ,     // Ifj
    TT_KEYWORD_STATIC,  // static
    TT_KEYWORD_IMPORT,  // import
    TT_KEYWORD_FOR,     // for
    TT_KEYWORD_NUM,     // num
    TT_KEYWORD_Null,    // Null

    // Operators
    TT_ASSIGN,          // =
    TT_PLUS,            // +
    TT_MINUS,           // -
    TT_MUL,             // *
    TT_DIV,             // /
    TT_EQ,              // ==
    TT_NEQ,             // !=
    TT_LT,              // <
    TT_GT,              // >
    TT_LE,              // <=
    TT_GE,              // >=

    // Symbols
    TT_LPAREN,          // (
    TT_RPAREN,          // )
    TT_LBRACE,          // {
    TT_RBRACE,          // }
    TT_COMMA,           // ,
    TT_DOT,             // .

    TT_ERROR            // error (unknown char) - might be replaces with just error message
} token_type_t;

typedef struct {
    token_type_t type;  // token type
    char *lexeme;       // orginial value (example: "while", "123", "x")
} token_t;

token_t get_token(void);

token_t make_token(token_type_t type, const char *lexeme);

token_type_t keyword_type(const char *lexeme);

static bool buf_append(char **buf, size_t *len, size_t *cap, char c);

#endif // SCANNER_H