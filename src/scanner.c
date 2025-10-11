// Implementace prekladace imperativniho jazyka IFJ25
// scanner.c by William Denis "xtihelw00" Tihelka on MM/DD/25.
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "scanner.h"

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
    STATE_LPAREN,         // Reading a left parenthesis
    STATE_RPAREN,         // Reading a right parenthesis
    STATE_LBRACE,         // Reading a left brace
    STATE_RBRACE,         // Reading a right brace
    STATE_EQ,             // Reading an equality operator
} State;


// Function to create a token with given type and lexeme
token_t make_token(token_type_t type, const char *lexeme) {
    token_t t;
    t.type = type;
    if (lexeme) {
        t.lexeme = malloc(strlen(lexeme) + 1); // memory allocation, +1 for null terminator
        if (t.lexeme) strcpy(t.lexeme, lexeme); // copy lexeme into token
        else {
            t.type = TT_ERROR; // if malloc fails, return error token, TO BE CHANGED ONCE PROPER ERROR HANDLING IS IMPLEMENTED
            t.lexeme = NULL;
        }
    } else {
        t.lexeme = NULL; // for tokens without lexeme (like TT_EOF, which doesnt work... yet)
    }
    return t;
}

// Helper function to append a character to a dynamic buffer, resizing if necessary
static bool buf_append(char **buf, size_t *len, size_t *cap, char c) {
    if (*len + 1 >= *cap) {
        size_t new_cap = *cap * 2;
        char *new_buf = realloc(*buf, new_cap);
        if (!new_buf) return false;
        *buf = new_buf;
        *cap = new_cap;
    }
    (*buf)[(*len)++] = c;
    (*buf)[*len] = '\0';
    return true;
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
    char *buf = NULL;
    size_t len = 0, cap = 64; // buffer length and capacity
    buf = malloc(cap);
    if (!buf) {
        return make_token(TT_ERROR, NULL); // memory allocation failed
    }
    while (1) {
        c = getchar();
        switch (state) { // FSM for tokenizing input
            case STATE_START:
                len = 0; // reset lexeme buffer for each new token
                if (c == EOF) {
                    free(buf);
                    return make_token(TT_EOF, NULL);
                }

                if (c == '\n' || c == '\r') {
                    free(buf);
                    return make_token(TT_EOL, "\\n");
                }

                if (isspace(c)) continue; // skip whitespace

                if (isalpha(c) || c == '_') {
                    if (!buf_append(&buf, &len, &cap, c)) {
                        free(buf);
                        return make_token(TT_ERROR, "realloc failed");
                    }
                    state = STATE_IDENTIFIER;
                } else if (isdigit(c)) {
                    if (!buf_append(&buf, &len, &cap, c)) {
                        free(buf);
                        return make_token(TT_ERROR, "realloc failed");
                    }
                    state = STATE_INT;
                } else if (c == '"') {
                    state = STATE_STRING;
                } else if (c == '/') {
                    state = STATE_SLASH;
                    break;
                } else {
                    // For now, treat any other char as error
                    if (!buf_append(&buf, &len, &cap, c)) {
                        free(buf);
                        return make_token(TT_ERROR, "realloc failed");
                    }
                    return make_token(TT_ERROR, buf); // unknown char, should be changed to proper error handling
                }
                break;

            case STATE_IDENTIFIER: // Identifiers and keywords
                if (isalnum(c) || c == '_') {
                    if (!buf_append(&buf, &len, &cap, c)) {
                        free(buf);
                        return make_token(TT_ERROR, "realloc failed");
                    }
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
                    if (!buf_append(&buf, &len, &cap, c)) {
                        free(buf);
                        return make_token(TT_ERROR, "realloc failed");
                    }
                } else if (c == '.') {
                    if (!buf_append(&buf, &len, &cap, c)) {
                        free(buf);
                        return make_token(TT_ERROR, "realloc failed");
                    }
                    state = STATE_FLOAT;
                } else {
                    buf[len] = '\0';
                    if (c != EOF) ungetc(c, stdin);
                    token_t tok = make_token(TT_INT, buf);
                    free(buf);
                    return tok;
                }
                break;

            case STATE_FLOAT: // Float literals
                if (isdigit(c)) {
                    if (!buf_append(&buf, &len, &cap, c)) {
                        free(buf);
                        return make_token(TT_ERROR, "realloc failed");
                    }
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
                        int c3 = getchar();
                        if (c3 == '"') {
                            // Enter multi-line string state
                            state = STATE_STRING_MULTI;
                            break;
                        } else {
                            // Only two quotes, treat as end of string and push back c3
                            if (c3 != EOF) ungetc(c3, stdin);
                            buf[len] = '\0';
                            token_t tok = make_token(TT_STRING, buf);
                            free(buf);
                            return tok;
                        }
                    } else {
                        // Only one quote, end of string, push back c2
                        if (c2 != EOF) ungetc(c2, stdin);
                        buf[len] = '\0';
                        token_t tok = make_token(TT_STRING, buf);
                        free(buf);
                        return tok;
                    }
                } else if (c == EOF) {
                    free(buf);
                    return make_token(TT_ERROR, NULL);
                } else {
                    if (!buf_append(&buf, &len, &cap, c)) {
                        free(buf);
                        return make_token(TT_ERROR, "realloc failed");
                    }
                }
                break;
            
            case STATE_STRING_MULTI: // Multi-line string literals (""" ... """)
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
                            // Not end, append chars and continue
                            if (!buf_append(&buf, &len, &cap, '"')) {
                                free(buf);
                                return make_token(TT_ERROR, "realloc failed");
                            }
                            if (!buf_append(&buf, &len, &cap, '"')) {
                                free(buf);
                                return make_token(TT_ERROR, "realloc failed");
                            }
                            if (c3 != EOF) {
                                if (!buf_append(&buf, &len, &cap, c3)) {
                                    free(buf);
                                    return make_token(TT_ERROR, "realloc failed");
                                }
                            }
                        }
                    } else {
                        // Not end, append quote and c2
                        if (!buf_append(&buf, &len, &cap, '"')) {
                            free(buf);
                            return make_token(TT_ERROR, "realloc failed");
                        }
                        if (c2 != EOF) {
                            if (!buf_append(&buf, &len, &cap, c2)) {
                                free(buf);
                                return make_token(TT_ERROR, "realloc failed");
                            }
                        }
                    }
                } else if (c == EOF) {
                    free(buf);
                    return make_token(TT_ERROR, NULL);
                } else {
                    if (!buf_append(&buf, &len, &cap, c)) {
                        free(buf);
                        return make_token(TT_ERROR, "realloc failed");
                    }
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
                    token_t tok = make_token(TT_DIV, buf);
                    free(buf);
                    return tok;
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
                        free(buf);
                        return make_token(TT_ERROR, NULL);
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

            case STATE_EQ:
                // To be implemented
                break;

            case STATE_LPAREN:
                // To be implemented        
                break;
            
            case STATE_RPAREN:
                // To be implemented
                break;      
            
            case STATE_LBRACE:
                // To be implemented    
                break;
            
            case STATE_RBRACE:
                // To be implemented
                break;  

        }
    }
}
