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
    STATE_START,         // Initial state, waiting for input
    STATE_IDENTIFIER,    // Reading an identifier or keyword
    STATE_INT,           // Reading an integer number
    STATE_FLOAT,         // Reading a floating-point number
    STATE_STRING,        // Reading a string literal
    STATE_SLASH,         // Reading a slash (could be division or start of comment)
    STATE_COMMENT_LINE,  // Reading a single-line comment
    STATE_COMMENT_BLOCK  // Reading a block comment
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
    char buf[256];  // buffer for lexeme
    int len = 0;   // length of lexeme
    while (1) {
        c = getchar();
        switch (state) { // FSM for tokenizing input
            case STATE_START:
                if (c == EOF) return make_token(TT_EOF, NULL);
                if (c == '\n' || c == '\r') {
                    return make_token(TT_EOL, "\\n");
                }
                if (isspace(c)) continue; // skip whitespace
                if (isalpha(c) || c == '_') {
                    buf[len++] = c;
                    state = STATE_IDENTIFIER;
                } else if (isdigit(c)) {
                    buf[len++] = c;
                    state = STATE_INT;
                } else if (c == '"') {
                    state = STATE_STRING;
                } else {
                    // For now, treat any other char as error
                    buf[0] = c;
                    buf[1] = '\0';
                    return make_token(TT_ERROR, buf); // unknown char, should be changed to proper error handling
                }
                break;

            case STATE_IDENTIFIER: // Identifiers and keywords
                if (isalnum(c) || c == '_') {
                    if (len < 255) buf[len++] = c;
                } else {
                    buf[len] = '\0';
                    if (c != EOF) ungetc(c, stdin);
                    token_type_t type = keyword_type(buf);
                    return make_token(type, buf);
                }
                break;

            case STATE_INT: // Integer literals only for now
                if (isdigit(c)) {
                    if (len < 255) buf[len++] = c;
                } else if (c == '.') {
                    if (len < 255) buf[len++] = c;
                    state = STATE_FLOAT;
                } else {
                    buf[len] = '\0';
                    if (c != EOF) ungetc(c, stdin);
                    return make_token(TT_INT, buf);
                }
                break;

            case STATE_FLOAT: // Float literals
                if (isdigit(c) || c == '.') {
                    if (len < 255) buf[len++] = c;
                } else {
                    buf[len] = '\0';
                    if (c != EOF) ungetc(c, stdin);
                    return make_token(TT_FLOAT, buf);
                }
                break;

            case STATE_STRING: // String literals
                if (c == '"') { // missing multiple line strings
                    buf[len] = '\0';
                    return make_token(TT_STRING, buf);
                } else if (c == EOF) {
                    return make_token(TT_ERROR, NULL);
                } else {
                    if (len < 255) buf[len++] = c;
                }
                break;

            case STATE_SLASH:
                // To be implemented
                break;

            case STATE_COMMENT_LINE:
                // To be implemented
                break;
            
            case STATE_COMMENT_BLOCK:
                // To be implemented
                break;

        }
    }
}
