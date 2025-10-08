#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"


token_t get_token();

int main(void) {
    token_t tok;
    while ((tok = get_token()).type != TT_EOF) {
        printf("TOKEN: type=%d, lexeme=%s\n", tok.type, tok.lexeme);
        free(tok.lexeme); // uvolnit paměť!
    }
    return 0;
}