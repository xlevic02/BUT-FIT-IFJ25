#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "scanner.h"
#include "ast.h"
#include "generator.h"
#include "symtable.h"


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

    bst_scope_ptr current_scope = NULL;
    ast_node_ptr ast_root = create_ast();

    visualize_ast(ast_root);

    generate_code(ast_root, current_scope);

    destroy_ast(ast_root);


    return 0;
}





