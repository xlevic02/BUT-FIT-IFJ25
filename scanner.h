// Implementace prekladace imperativniho jazyka IFJ25
// scanner.h by William Denis "xtihelw00" Tihelka on MM/DD/25.
//
typedef enum {
    TOKEN_EOF,             // end of file
    TOKEN_EOL,             // end of line

    TOKEN_IDENTIFIER,      // identifier
    TOKEN_KEYWORD,         // keyword(while,if,...)
    TOKEN_NULL,            // null literal

    TOKEN_INT_LITERAL,     // intiger
    TOKEN_FLOAT_LITERAL,   // float
    TOKEN_STRING_LITERAL,  // string

    // Operátory
    TOKEN_ASSIGN,          // =
    TOKEN_OPERATOR_PLUS,   // +
    TOKEN_OPERATOR_MINUS,  // -
    TOKEN_OPERATOR_MUL,    // *
    TOKEN_OPERATOR_DIV,    // /
    TOKEN_OPERATOR_EQ,     // ==
    TOKEN_OPERATOR_NEQ,    // !=
    TOKEN_OPERATOR_LT,     // <
    TOKEN_OPERATOR_GT,     // >
    TOKEN_OPERATOR_LE,     // <=
    TOKEN_OPERATOR_GE,     // >=
    TOKEN_OPERATOR_IS,     // is

    // Oddělovače a symboly
    TOKEN_LPAREN,          // (
    TOKEN_RPAREN,          // )
    TOKEN_LBRACE,          // {
    TOKEN_RBRACE,          // }
    TOKEN_COMMA,           // ,
    TOKEN_DOT,             // .

    TOKEN_ERROR            // error (unknown char) - might be replaces with just error message
} TokenType;

typedef struct {
    TokenType type;   // token type
    char *lexeme;     // orginial value (example: "while", "123", "x")
} Token;