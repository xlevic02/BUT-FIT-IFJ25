// Implementace prekladace imperativniho jazyka IFJ25
// Semantic analysis by Jan Frantisek "xlevic02" Levicek on 11/15/25
//

#include "sem_analysis.h"


bst_scope_ptr sem_start_analysis(ast_node_ptr root) {
    bst_scope_ptr global = NULL;
    if(bst_increase_scope(&global)) {
        bst_destroy_symbol_table(global);
        ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, root, NULL);
    }

    for(int i = 0; i < root->n_of_children; i++) {

        switch(bst_define_variable(global, root->children[i])) {
            case 0:
                break;

            case ERROR_INTERNAL:
                bst_free_scope(global);
                ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, root, NULL);

            case ERROR_SEM_REDEF:
                bst_free_scope(global);
                ast_error(ERROR_SEM_REDEF, MSG_SEM_REDEFINING, root, NULL);
        }

        sem_func_eval(root->children[i], global);
    }
}


void sem_func_eval(ast_node_ptr func, bst_scope_ptr parent_scope) {
    if(bst_increase_scope(&parent_scope)) {
        bst_free_scope(parent_scope);
        ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, func, NULL);
    }

    bst_scope_ptr func_scope = parent_scope->children[parent_scope->n_of_children - 1];

    for(int i = 0; i < func->n_of_children; i++) {
        switch(func->children[i]->node_type) {
            case NT_VAR_DEFINITION:
                bst_define_variable(func_scope, func->children[i]);
                break;

            case NT_ASSIGN:
                bst_declare_variable(func_scope, func->children[i]->children[0], func->children[i]->children[1]);


            default:
                break;
        }
    }
}