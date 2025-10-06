// Implementace prekladace imperativniho jazyka IFJ25
// scanner.h by William Denis "xtihelw00" Tihelka on MM/DD/25.
//
typedef enum {
    TT_EOF,             // end of file
    TT_EOL,             // end of line

    TT_IDENTIFIER,      // identifier
    TT_NULL,            // null literal
    TT_INT,             // intiger
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
} TokenType;

typedef struct {
    TokenType type;   // token type
    char *lexeme;     // orginial value (example: "while", "123", "x")
} Token;