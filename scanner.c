// Implementace prekladace imperativniho jazyka IFJ25
// scanner.c by William Denis "xtihelw00" Tihelka on MM/DD/25.
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"

typedef enum {
    STATE_START,
    STATE_IDENTIFIER,
    STATE_NUMBER,
    STATE_STRING
} State;


// pomocná funkce pro vytvoření tokenu
token_t make_token(token_type_t type, const char *lexeme) {
    token_t t;
    t.type = type;
    if (lexeme) {
        t.lexeme = malloc(strlen(lexeme) + 1);
        if (t.lexeme) strcpy(t.lexeme, lexeme);
    } else {
        t.lexeme = NULL;
    }
    return t;
}

token_t get_token() { // this is just a prototype, nothing is functional, i was just visualizing how the state stuff works
    State state = STATE_START;
    int c;
    char buf[256];
    int len = 0;
    while (1) {
        c = getchar();
        switch (state) {
            case STATE_START:
                if (c == EOF) return make_token(TT_EOF, NULL);
                if (isspace(c)) continue;
                if (isalpha(c) || c == '_') {
                    buf[len++] = c;
                    state = STATE_IDENTIFIER;
                } else if (isdigit(c)) {
                    buf[len++] = c;
                    state = STATE_NUMBER;
                } else if (c == '"') {
                    state = STATE_STRING;
                } else {
                    // For now, treat any other char as error
                    buf[0] = c;
                    buf[1] = '\0';
                    return make_token(TT_ERROR, buf);
                }
                break;

            case STATE_IDENTIFIER:
                if (isalnum(c) || c == '_') {
                    if (len < 255) buf[len++] = c;
                } else {
                    buf[len] = '\0';
                    if (c != EOF) ungetc(c, stdin);
                    return make_token(TT_IDENTIFIER, buf);
                }
                break;

            case STATE_NUMBER:
                if (isdigit(c)) {
                    if (len < 255) buf[len++] = c;
                } else {
                    buf[len] = '\0';
                    if (c != EOF) ungetc(c, stdin);
                    return make_token(TT_INT, buf);
                }
                break;

            case STATE_STRING:
                if (c == '"') {
                    buf[len] = '\0';
                    return make_token(TT_STRING, buf);
                } else if (c == EOF) {
                    return make_token(TT_ERROR, NULL);
                } else {
                    if (len < 255) buf[len++] = c;
                }
                break;
        }
    }
}
