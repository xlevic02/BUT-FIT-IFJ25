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
    if (!node) {
        printf("%s%s NULL NODE\n", prefix, is_last ? "└── " : "├── ");
        return;
    }

    // Check if pointer looks valid (not a common invalid value)
    if ((unsigned long)node < 0x1000) {
        printf("%s%s INVALID POINTER: %p\n", prefix, is_last ? "└── " : "├── ", (void*)node);
        return;
    }

    printf("%s%s ", prefix, is_last ? "└── " : "├── ");
    fflush(stdout);

    // Try to access token.type
    token_type_t type;
    type = node->token.type;
    printf("%s", token_type_to_string(type));
    fflush(stdout);

    // Try to access lexeme
    if (node->token.lexeme) {
        printf(" ('%s')", node->token.lexeme);
    }
    fflush(stdout);

    // Try to access n_of_children
    int n_children = node->n_of_children;
    printf(" [children: %d]", n_children);
    fflush(stdout);

    printf("\n");
    fflush(stdout);

    // Prepare new prefix
    char new_prefix[512];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_last ? "    " : "│   ");

    // Check children pointer
    if (n_children > 0 && node->children == NULL) {
        printf("%sERROR: n_of_children=%d but children is NULL\n", new_prefix, n_children);
        return;
    }

    // Recursively print children
    for (int i = 0; i < n_children; i++) {
        printf("%sDEBUG: Child %d/%d at %p\n", new_prefix, i+1, n_children, (void*)node->children[i]);
        fflush(stdout);
        print_ast_branch(node->children[i], new_prefix, i == n_children - 1);
    }
}

// --- Entry function ---
void visualize_ast(const ast_node_ptr root) {
    printf("🌳 Abstract Syntax Tree\n");
    fflush(stdout);
    if (root == NULL) {
        printf("ROOT IS NULL!\n");
        return;
    }
    printf("Root at %p\n", (void*)root);
    fflush(stdout);
    print_ast_branch(root, "", 1);
    printf("\nVisualization complete!\n");
    fflush(stdout);
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

    free_ast(ast_root);

    printf(ast_root == NULL ? "AST successfully freed.\n" : "AST freeing failed!\n");
    ast_print_token(ast_root->token);
    destroy_ast(ast_root);


    return 0;
}





