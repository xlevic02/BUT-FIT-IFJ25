// Implementace prekladace imperativniho jazyka IFJ25
// scanner.c by William Denis "xtihelw00" Tihelka on 10/13/25.
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "scanner.h"
#include "error.h"

// States for the deterministic finite state machine (DFSM) used in the scanner
typedef enum {
    STATE_START,          // Initial state, waiting for input
    STATE_IDENTIFIER,     // Reading an identifier or keyword
    STATE_INT,            // Reading an integer number
    STATE_FLOAT,          // Reading a floating-point number
    STATE_STRING,         // Reading a string literal
    STATE_SLASH,          // Reading a slash (could be division or start of comment)
    STATE_COMMENT_LINE,   // Reading a single-line comment
    STATE_COMMENT_BLOCK,  // Reading a block comment
    STATE_STRING_MULTI,   // Reading a multi-line string
    STATE_EQ,             // Reading an equality operator
    STATE_NEQ,            // Reading a not-equal operator
    STATE_LT,             // Reading a less-than operator   
    STATE_GT,             // Reading a greater-than operator
    STATE_DOT,            // Reading a dot
} State;


// Function to create a token with given type and lexeme
token_t make_token(token_type_t type, const char *lexeme) {
    token_t t;
    t.type = type;

    if (lexeme) {
        t.lexeme = malloc(strlen(lexeme) + 1);
        if (!t.lexeme)
            error(ERROR_INTERNAL, MSG_GEN_INTERNAL);  // Exits immediately

        strcpy(t.lexeme, lexeme);
    } else {
        t.lexeme = NULL;
    }

    return t;
}


// Helper function to append a character to a dynamic buffer, resizing if necessary
void buf_append(char **buf, size_t *len, size_t *cap, char c) {
    if (*len + 1 >= *cap) {
        size_t new_cap = *cap * 2;
        char *new_buf = realloc(*buf, new_cap);
        if (!new_buf) error(ERROR_INTERNAL, MSG_GEN_INTERNAL);
        *buf = new_buf;
        *cap = new_cap;
    }
    (*buf)[(*len)++] = c;
    (*buf)[*len] = '\0';
}

// Helper function to check if a string is a keyword and return its token_type_t, extremely inefficient but works for now
// Returns TT_IDENTIFIER if not a keyword
token_type_t keyword_type(const char *lexeme) { 
    if (strcmp(lexeme, "class") == 0) return TT_KEYWORD_CLASS;
    if (strcmp(lexeme, "if") == 0) return TT_KEYWORD_IF;
    if (strcmp(lexeme, "else") == 0) return TT_KEYWORD_ELSE;
    if (strcmp(lexeme, "is") == 0) return TT_KEYWORD_IS;
    if (strcmp(lexeme, "return") == 0) return TT_KEYWORD_RETURN;
    if (strcmp(lexeme, "var") == 0) return TT_KEYWORD_VAR;
    if (strcmp(lexeme, "while") == 0) return TT_KEYWORD_WHILE;
    if (strcmp(lexeme, "Ifj") == 0) return TT_KEYWORD_IFJ;
    if (strcmp(lexeme, "static") == 0) return TT_KEYWORD_STATIC;
    if (strcmp(lexeme, "import") == 0) return TT_KEYWORD_IMPORT;
    if (strcmp(lexeme, "for") == 0) return TT_KEYWORD_FOR;
    if (strcmp(lexeme, "num") == 0) return TT_KEYWORD_NUM;
    if (strcmp(lexeme, "Null") == 0) return TT_KEYWORD_Null;
    return TT_IDENTIFIER; // not a keyword, return identifier type
}

// Main scanner function implementing a deterministic finite state machine (DFSM)
// Reads input character by character and transitions between states to build tokens
token_t get_token() { // First "functional" version of scanner with basic tokens, more to be added
    State state = STATE_START; // initial state
    int c;      // current character
    char *buf =  malloc(INITIAL_BUF_CAP);
    size_t len = 0, cap = INITIAL_BUF_CAP; // buffer length and capacity
    if (!buf) {
        error(ERROR_INTERNAL, MSG_GEN_INTERNAL); // memory allocation failed
    }

    while (1) {
        c = getchar();

        switch (state) { // FSM for tokenizing input
            case STATE_START:
                len = 0; // reset lexeme buffer for each new token
                buf[0] = '\0';

                if (c == EOF) {
                    free(buf);
                    return make_token(TT_EOF, NULL);
                }

                if (c == '\r') {
                    int next = getchar();
                    if (next != '\n' && next != EOF) ungetc(next, stdin);
                    free(buf);
                    return make_token(TT_EOL, "\\n");
                }
                if (c == '\n') {
                    free(buf);
                    return make_token(TT_EOL, "\\n");
                }

                if (isspace(c)) continue; // skip whitespace

                // Identifiers and keywords
                if (isalpha(c) || c == '_') {
                    buf_append(&buf, &len, &cap, c);
                    state = STATE_IDENTIFIER;
                } 
                // Numbers
                else if (isdigit(c)) {
                    buf_append(&buf, &len, &cap, c);
                    state = STATE_INT;
                }
                // String literals
                else if (c == '"') {
                    state = STATE_STRING;
                    break;
                }
                
                else if (c == '/') state = STATE_SLASH;
                else if (c == '=') state = STATE_EQ;
                else if (c == '!') state = STATE_NEQ;
                else if (c == '<') state = STATE_LT;
                else if (c == '>') state = STATE_GT;
                else if (c == '.') state = STATE_DOT;
                else if (c == '+') {
                    free(buf);
                    return make_token(TT_PLUS, "+");
                }
                else if (c == '-') {
                    free(buf);
                    return make_token(TT_MINUS, "-");
                }
                else if (c == '*') {
                    free(buf);
                    return make_token(TT_MUL, "*");
                }
                else if (c == '(') {
                    free(buf);
                    return make_token(TT_LPAREN, "(");
                }
                else if (c == ')') {
                    free(buf);
                    return make_token(TT_RPAREN, ")");
                }
                else if (c == '{') {
                    free(buf);
                    return make_token(TT_LBRACE, "{");
                }
                else if (c == '}') {
                    free(buf);
                    return make_token(TT_RBRACE, "}");
                }
                else if (c == ',') {
                    free(buf);
                    return make_token(TT_COMMA, ",");
                }
                else {
                    // For now, treat any other char as error
                    buf_append(&buf, &len, &cap, c);
                    error(ERROR_LEXICAL, MSG_LEX_PROHIBITED_CHAR);
                    return make_token(TT_ERROR, buf);
                }
                break;

            case STATE_EQ:
                if (c == '=') {
                    free(buf);
                    return make_token(TT_EQ, "==");
                } else {
                    if (c != EOF) ungetc(c, stdin);
                    token_t tok = make_token(TT_ASSIGN, "=");
                    free(buf);
                    return tok;
                }
                break;

            case STATE_NEQ:
                if (c == '=') {
                    free(buf);
                    return make_token(TT_NEQ, "!=");
                } else {
                    // Just '!' is not a valid operator in IFJ25, treat as error
                    if (c != EOF) ungetc(c, stdin);
                    token_t tok = make_token(TT_ERROR, "!");
                    free(buf);
                    return tok;
                }
                break;

            case STATE_LT:
                if (c == '=') {
                    token_t tok = make_token(TT_LE, "<=");
                    free(buf);
                    return tok;
                } else {
                    if (c != EOF) ungetc(c, stdin);
                    free(buf);
                    return make_token(TT_LT, "<");
                }
                break;

            case STATE_GT:
                if (c == '=') {
                    token_t tok = make_token(TT_GE, ">=");
                    free(buf);
                    return tok;
                } else {
                    if (c != EOF) ungetc(c, stdin);
                    free(buf);
                    return make_token(TT_GT, ">");
                }
                break;

            case STATE_IDENTIFIER: // Identifiers and keywords
                if (isalnum(c) || c == '_') {
                    buf_append(&buf, &len, &cap, c);
                } else {
                    buf[len] = '\0';
                    if (c != EOF) ungetc(c, stdin);
                    token_type_t type = keyword_type(buf);
                    token_t tok = make_token(type, buf);
                    free(buf);
                    return tok;
                }
                break;

            case STATE_INT: // Integer literals
                if (isdigit(c)) {
                    buf_append(&buf, &len, &cap, c);

                } else if (c == '.') { // Loaded decimal point, switch to float state
                    buf_append(&buf, &len, &cap, c);
                    
                    state = STATE_FLOAT;
                } else { // Whitespace or other char, end of integer
                    buf[len] = '\0';
                    if (c != EOF) ungetc(c, stdin);
                    token_t tok = make_token(TT_INT, buf);
                    free(buf);
                    return tok;
                }
                break;

            case STATE_FLOAT: // Float literals
                if (isdigit(c)) {
                    buf_append(&buf, &len, &cap, c);

                } else {
                    buf[len] = '\0';
                    if (c != EOF) ungetc(c, stdin);
                    token_t tok = make_token(TT_FLOAT, buf);
                    free(buf);
                    return tok;
                }
                break;

            case STATE_STRING: // String literals
                if (c == '"') {
                    // Check for triple quote (multi-line string)
                    int c2 = getchar();
                    if (c2 == '"') {
                        // Enter multi-line string state
                        state = STATE_STRING_MULTI;
                        break;
                    } else {
                        // Only two quotes, treat as end of string and push back c2
                        if (c2 != EOF) ungetc(c2, stdin);
                        buf[len] = '\0';
                        token_t tok = make_token(TT_STRING, buf);
                        free(buf);
                        return tok;
                    }
                } else if (c == EOF) {
                    free(buf);
                    error(ERROR_LEXICAL, MSG_LEX_UNCLOSED_STRING);
                } else {
                    buf_append(&buf, &len, &cap, c);
                }
                break;

            case STATE_STRING_MULTI: // Multi-line string literals (""" ... """)
                if (c == EOF) {
                    free(buf);
                    error(ERROR_LEXICAL, MSG_LEX_UNCLOSED_STRING);
                }

                if (c == '"') {
                    int c2 = getchar();
                    if (c2 == '"') {
                        int c3 = getchar();
                        if (c3 == '"') {
                            // End of multi-line string
                            buf[len] = '\0';
                            token_t tok = make_token(TT_STRING, buf);
                            free(buf);
                            return tok;
                        } else {
                        // Not end, append chars and continue reading
                        buf_append(&buf, &len, &cap, '"');
                        buf_append(&buf, &len, &cap, '"');
                        if (c3 != EOF) buf_append(&buf, &len, &cap, c3);
                        }
                    } else {
                        // Only one quote, not a triple
                        buf_append(&buf, &len, &cap, '"');
                        if (c2 != EOF) buf_append(&buf, &len, &cap, c2);
                    }
                } else {
                    // Normal character inside multi-line string
                    buf_append(&buf, &len, &cap, c);
                }
                break;


            case STATE_SLASH:
                if (c == '/') {
                    state = STATE_COMMENT_LINE;
                    break;
                } else if (c == '*') {
                    state = STATE_COMMENT_BLOCK;
                    break;
                } else {
                    // It's just a division operator
                    buf[0] = '/'; buf[1] = '\0';
                    if (c != EOF) ungetc(c, stdin);
                    free(buf);
                    return make_token(TT_DIV, "/");
                }
                break;

            case STATE_COMMENT_LINE:
                // Skip until end of line or EOF
                while ((c = getchar()) != EOF && c != '\n' && c != '\r');
                state = STATE_START;
                break;

            case STATE_COMMENT_BLOCK:
                // Skip until closing */ or EOF
                while (1) {
                    c = getchar();
                    if (c == EOF) {
                        error(ERROR_LEXICAL,MSG_LEX_UNCLOSED_COMMENT);
                    }
                    if (c == '*') {
                        int next = getchar();
                        if (next == '/') {
                            state = STATE_START;
                            break;
                        } else if (next != EOF) {
                            ungetc(next, stdin);
                        }
                    }
                }
                break;

            case STATE_DOT:
                ungetc(*buf, stdin);
                free(buf);
                return make_token(TT_DOT, "."); 
                break;

        }
    }
}
    