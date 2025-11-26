#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "scanner.h"
#include "ast.h"
#include "symtable.h"
#include "sem_analysis.h"

const char* token_type_to_string(token_type_t type);
void ast_print_token(token_t token);

//REMOVE, FOR TESTING PURPOSES ONLY

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
    printf("ast ok\n");
    bst_scope_ptr global_scope = sem_start_analysis(ast_root);
    printf("symtable ok\n");

    return 0;
    visualize_ast(ast_root);

    free_ast(ast_root);

    printf(ast_root == NULL ? "AST successfully freed.\n" : "AST freeing failed!\n");
    ast_print_token(ast_root->token);

    return 0;
}

void ast_print_token(token_t token){
    printf("Token type: %s, lexeme: %s\n", token_type_to_string(token.type), token.lexeme);
}

const char* token_type_to_string(token_type_t type) {
    switch (type) {
        case TT_EOF: return "TT_EOF";
        case TT_EOL: return "TT_EOL";

        case TT_IDENTIFIER: return "TT_IDENTIFIER";
        case TT_NULL: return "TT_NULL";
        case TT_INT: return "TT_INT";
        case TT_FLOAT: return "TT_FLOAT";
        case TT_STRING: return "TT_STRING";

        case TT_KEYWORD_CLASS: return "TT_KEYWORD_CLASS";
        case TT_KEYWORD_IF: return "TT_KEYWORD_IF";
        case TT_KEYWORD_ELSE: return "TT_KEYWORD_ELSE";
        case TT_KEYWORD_IS: return "TT_KEYWORD_IS";
        case TT_KEYWORD_RETURN: return "TT_KEYWORD_RETURN";
        case TT_KEYWORD_VAR: return "TT_KEYWORD_VAR";
        case TT_KEYWORD_WHILE: return "TT_KEYWORD_WHILE";
        case TT_KEYWORD_IFJ: return "TT_KEYWORD_IFJ";
        case TT_KEYWORD_STATIC: return "TT_KEYWORD_STATIC";
        case TT_KEYWORD_IMPORT: return "TT_KEYWORD_IMPORT";
        case TT_KEYWORD_FOR: return "TT_KEYWORD_FOR";
        case TT_KEYWORD_NUM: return "TT_KEYWORD_NUM";
        case TT_KEYWORD_Null: return "TT_KEYWORD_Null";

        case TT_ASSIGN: return "TT_ASSIGN";
        case TT_PLUS: return "TT_PLUS";
        case TT_MINUS: return "TT_MINUS";
        case TT_MUL: return "TT_MUL";
        case TT_DIV: return "TT_DIV";
        case TT_EQ: return "TT_EQ";
        case TT_NEQ: return "TT_NEQ";
        case TT_LT: return "TT_LT";
        case TT_GT: return "TT_GT";
        case TT_LE: return "TT_LE";
        case TT_GE: return "TT_GE";

        case TT_LPAREN: return "TT_LPAREN";
        case TT_RPAREN: return "TT_RPAREN";
        case TT_LBRACE: return "TT_LBRACE";
        case TT_RBRACE: return "TT_RBRACE";
        case TT_COMMA: return "TT_COMMA";
        case TT_DOT: return "TT_DOT";

        case TT_ERROR: return "TT_ERROR";

        default: return "UNKNOWN_TOKEN";
    }
}


