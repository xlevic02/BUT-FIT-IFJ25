// Implementace prekladace imperativniho jazyka IFJ25
// scanner.c by William Denis "xtihelw00" Tihelka on MM/DD/25.
//

#include <ctype.h>
#include <stdio.h>
#include "scanner.h"

typedef enum {
    STATE_START,
    STATE_IDENTIFIER,
    STATE_NUMBER,
    STATE_STRING,
    STATE_COMMENT_LINE,
    STATE_COMMENT_BLOCK,
    STATE_DONE,
    STATE_ERROR
} State;

Token get_token() { //this is just a prototype, nothing is funtional, i was just visualizing how the state stuff works 
    State state = STATE_START;
    int c;
    while ((c = getchar()) != EOF) {
        switch (state) {
            case STATE_START:
                if (isalpha(c) || c == '_') state = STATE_IDENTIFIER;
                else if (isdigit(c)) state = STATE_NUMBER;
                else if (c == '"') state = STATE_STRING;
                else if (isspace(c)) continue; 
                else return make_token(TOKEN_ERROR, NULL);
                break;

            case STATE_IDENTIFIER:
                if (!isalnum(c) && c != '_') {
                    ungetc(c, stdin); 
                    return make_identifier_or_keyword();
                }
                break;
            
            
        }
    }
    return make_token(TOKEN_EOF, NULL);
}
