#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "scanner.h"
#include "ast.h"


//REMOVE, FOR TESTING PURPOSES ONLY

// --- Helper: convert token type to readable name ---
const char* token_type_to_string(int type) {
    switch (type) {
        case 0: return "EOF";
        case 1: return "EOL";
        case 2: return "IDENTIFIER";
        case 3: return "NULL";
        case 4: return "INT";
        case 5: return "FLOAT";
        case 6: return "STRING";
        case 7: return "KEYWORD_CLASS";
        case 8: return "KEYWORD_IF";
        case 9: return "KEYWORD_ELSE";
        case 10: return "KEYWORD_IS";
        case 11: return "KEYWORD_RETURN";
        case 12: return "KEYWORD_VAR";
        case 13: return "KEYWORD_WHILE";
        case 14: return "KEYWORD_IFJ";
        case 15: return "KEYWORD_STATIC";
        case 16: return "KEYWORD_IMPORT";
        case 17: return "KEYWORD_FOR";
        case 18: return "KEYWORD_NUM";
        case 19: return "KEYWORD_Null";
        case 20: return "ASSIGN";
        case 21: return "PLUS";
        case 22: return "MINUS";
        case 23: return "MUL";
        case 24: return "DIV";
        case 25: return "EQ";
        case 26: return "NEQ";
        case 27: return "LT";
        case 28: return "GT";
        case 29: return "LE";
        case 30: return "GE";
        case 31: return "LPAREN";
        case 32: return "RPAREN";
        case 33: return "LBRACE";
        case 34: return "RBRACE";
        case 35: return "COMMA";
        case 36: return "DOT";
        case 37: return "ERROR";
        default: return "UNKNOWN";
    }
}

// --- Recursive visualizer ---
void print_ast_branch(const ast_node_ptr node, const char *prefix, int is_last) {
    if (!node) return;

    printf("%s", prefix);
    printf(is_last ? "└── " : "├── ");

    // Print node content
    printf("%s", token_type_to_string(node->token.type));
    if (node->token.lexeme)
        printf(" ('%s')", node->token.lexeme);

    printf("\n");

    // Prepare new prefix for children
    char new_prefix[512];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_last ? "    " : "│   ");

    // Recursively print children
    for (int i = 0; i < node->n_of_children; i++) {
        print_ast_branch(node->children[i], new_prefix, i == node->n_of_children - 1);
    }
}

// --- Entry function ---
void visualize_ast(const ast_node_ptr root) {
    printf("🌳 Abstract Syntax Tree\n");
    print_ast_branch(root, "", 1);
    printf("\n");
}

int main(void) {

    /*
    token_t tok;
    while ((tok = get_token()).type != TT_EOF) {
        printf("TOKEN: type=%d, lexeme=\"%s\"\n", tok.type, tok.lexeme);
        free(tok.lexeme); // FREE THE MEMORY!
    }
    printf("TOKEN: type=%d, lexeme=\"%s\"\n", tok.type, tok.lexeme);
     */

    ast_node_ptr ast_root = create_ast();

    visualize_ast(ast_root);

    destroy_ast(ast_root);

    return 0;
}





