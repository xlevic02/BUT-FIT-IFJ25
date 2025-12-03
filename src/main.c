// Implementace prekladace imperativniho jazyka IFJ25
// MAIN by Jan Frantisek "xlevic02" Levicek on 12/03/2025
#include <stdio.h>
#include "ast.h"
#include "symtable.h"
#include "sem_analysis.h"
#include "generator.h"



int main(void) {
    fflush(stdout);
    ast_node_ptr ast_root = create_ast();
    bst_scope_ptr global_scope = sem_start_analysis(ast_root);
    generate_code(ast_root, global_scope);

    bst_destroy_symbol_table(global_scope);
    destroy_ast(ast_root);

    return 0;
}
