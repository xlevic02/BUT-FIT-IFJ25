// Implementace prekladace imperativniho jazyka IFJ25
// Semantic analysis by Jan Frantisek "xlevic02" Levicek on 11/15/25
//

#include "sem_analysis.h"


bst_scope_ptr sem_start_analysis(ast_node_ptr root) {
    int main_index = -1;
    bool found_main = false;
    bst_scope_ptr global = NULL;
    if (bst_increase_scope(&global)) {
        bst_destroy_symbol_table(global);
        ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, root, NULL);
    }

    for (int i = 0; i < root->n_of_children; i++) {

        switch (bst_define_variable(global, root->children[i])) {
            case 0:
                break;

            case ERROR_INTERNAL:
                bst_free_scope(global);
                ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, root, NULL);

            case ERROR_SEM_REDEF:
                bst_free_scope(global);
                ast_error(ERROR_SEM_REDEF, MSG_SEM_REDEF, root, NULL);
        }

        if (strcmp(root->children[i]->token.lexeme, "main") == 0) {
            if (!found_main && main_index == -1) {
                main_index = i;
                found_main = true;
            } else {
                if (root->children[main_index]->children[0]->n_of_children != 0) { // if the num of params of the last possible main func IS NOT 0
                    if (root->children[i]->children[0]->n_of_children != 0) {      // if the num of params of the new possible main func IS NOT 0
                        found_main = false;
                    } else {
                        main_index = i;
                        found_main = true;
                    }
                }
            }
        }
    }

    if (found_main == false) {
        bst_free_scope(global);
        ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_MAIN, root, NULL);
    }

    sem_func_declare(root->children[main_index], global);
    return global;
}


void sem_func_declare(ast_node_ptr func, bst_scope_ptr global) {
    if (func == NULL) {
        bst_free_scope(global);
        ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, NULL, NULL);
    }

    if (bst_increase_scope(&global)) {
        bst_free_scope(global);
        ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, func, NULL);
    }

    bst_scope_ptr func_scope = global->children[global->n_of_children - 1];
    func_scope->key = get_hash(func);
    if (func_scope->key == 0) {
        bst_free_scope(global);
        ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, func, NULL);
    }

    sem_block_eval(func_scope, func->children[1]);
    bst_decrease_scope(global->tree);
}

void sem_block_eval(bst_scope_ptr scope, ast_node_ptr block_node) {
    bst_scope_ptr old_scope = scope;
    bst_value_node_ptr new_node = NULL;
    unsigned int key;

    for (int i = 0; i < block_node->n_of_children; i++) {
        switch (block_node->children[i]->node_type) {
            case NT_VAR_DEF:
                if(strncmp(block_node->children[i]->token.lexeme, "__", 2) == 0)
                    while(scope->parent != NULL)
                        scope = scope->parent;

                bst_define_variable(scope, block_node->children[i]);
                scope = old_scope;
                break;

            case NT_ASSIGN:
                key = get_hash(block_node->children[i]->children[0]);
                if (key == 0) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, block_node, NULL);
                }

                bst_node_ptr old_node = bst_search(scope->tree, key);
                if (old_node == NULL) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, block_node, NULL);
                }

                if(old_node->content.type == NT_SETTER) {
                    while(scope->parent != NULL)
                        scope = scope->parent;

                    //sem_func_eval(); TODO

                    scope = old_scope;
                    break;
                }


                new_node = old_node->content.value_tree->parent;
                new_node->children = realloc(old_node->content.value_tree->parent->children,
                                   sizeof(bst_value_node_ptr) * ++new_node->n_of_children);
                if(new_node->children == NULL) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_REALLOC, block_node, NULL);
                }

                new_node->children[new_node->n_of_children - 1] = malloc(sizeof(bst_value_node_t));
                if(new_node->children[new_node->n_of_children - 1] == NULL) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, block_node, NULL);
                }

                new_node->children[new_node->n_of_children - 1]->parent = new_node;
                new_node->children[new_node->n_of_children - 1]->children = NULL;
                new_node->children[new_node->n_of_children - 1]->n_of_children = 0;
                new_node->children[new_node->n_of_children - 1]->current_index = -1;
                new_node->current_index = new_node->n_of_children - 1;

                old_node->content.value_tree = new_node->children[new_node->n_of_children - 1];

                old_node->content.value_tree->value = sem_expr_type_eval(scope, block_node->children[i]->children[1]);

                break;

            case NT_IF_STATEMENT:
                sem_expr_type_eval(scope, block_node->children[i]->children[0]);

                if(bst_increase_scope(&scope)) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, block_node, NULL);
                }

                sem_block_eval(scope->children[scope->n_of_children - 1], block_node->children[1]);
                while(scope->parent != NULL) {
                    bst_decrease_scope(scope->tree);
                    scope = scope->parent;
                }

                scope = old_scope;

                if(bst_increase_scope(&scope)) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, block_node, NULL);
                }

                sem_block_eval(scope->children[scope->n_of_children - 1], block_node->children[i]->children[2]);
                while(scope->parent != NULL) {
                    bst_decrease_scope(scope->tree);
                    scope = scope->parent;
                }

                break;

            case NT_WHILE:
                sem_expr_type_eval(scope, block_node->children[i]->children[0]);

                if(bst_increase_scope(&scope)) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, block_node, NULL);
                }

                sem_block_eval(scope->children[scope->n_of_children - 1], block_node->children[i]->children[1]);
                while(scope->parent != NULL) {
                    bst_decrease_scope(scope->tree);
                    scope = scope->parent;
                }

                break;

            case NT_BLOCK:
                if(bst_increase_scope(&scope)) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, block_node, NULL);
                }

                sem_block_eval(scope->children[scope->n_of_children - 1], block_node->children[i]->children[0]);
                while(scope->parent != NULL) {
                    bst_decrease_scope(scope->tree);
                    scope = scope->parent;
                }

                break;

            case NT_RETURN:
                while(block_node->node_type != NT_FUNC_DECL)
                    block_node = block_node->parent;

                while(scope->parent != NULL)
                    scope = scope->parent;

                key = get_hash(block_node);
                if(key == 0) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, block_node, NULL);
                }

                bst_node_ptr func_node = bst_search(scope->tree, key);

                new_node = func_node->content.value_tree->parent;
                new_node->children = realloc(func_node->content.value_tree->parent->children,
                                             sizeof(bst_value_node_ptr) * ++new_node->n_of_children);
                if(new_node->children == NULL) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_REALLOC, block_node, NULL);
                }

                new_node->children[new_node->n_of_children - 1] = malloc(sizeof(bst_value_node_t));
                if(new_node->children[new_node->n_of_children - 1] == NULL) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, block_node, NULL);
                }

                new_node->children[new_node->n_of_children - 1]->parent = new_node;
                new_node->children[new_node->n_of_children - 1]->children = NULL;
                new_node->children[new_node->n_of_children - 1]->current_index = -1;

                func_node->content.value_tree = new_node->children[new_node->n_of_children - 1];
                func_node->content.value_tree->parent->current_index = func_node->content.value_tree->parent->n_of_children - 1;

                func_node->content.value_tree->value = sem_expr_type_eval(scope, block_node->children[i]->children[1]);
                scope = old_scope;

                return;

            default:
                break;
        }
    }
}


token_type_t sem_expr_type_eval(bst_scope_ptr scope, ast_node_ptr expr_node) {
    token_type_t left;
    token_type_t right;
    bst_node_ptr term = NULL;
    bst_scope_ptr func_scope = NULL;
    unsigned int key;

    key = get_hash(expr_node);
    if (key == 0) {
        bst_destroy_symbol_table(scope);
        ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, expr_node, NULL);
    }

    bst_scope_ptr global = scope;
    while (global->parent != NULL)
        global = global->parent;

    switch (expr_node->n_of_children) {
        // var or getter
        case 0:
            if (expr_node->node_type == NT_ID) {
                term = bst_search(scope->tree, key);
                while (term == NULL) {
                    if (scope->parent == NULL) {
                        if (strncmp(expr_node->token.lexeme, "__", 2)) {
                            bst_free_scope(global);
                            ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, expr_node, NULL);
                        } else
                            return TT_NULL;
                    }

                    scope = scope->parent;
                    term = bst_search(scope->tree, key);
                }

                if (term->content.type == NT_GETTER) {
                    if (term->content.value_tree == NULL)
                        sem_func_declare(sem_find_func(expr_node), global);


                    return sem_func_eval(global, expr_node);
                }

                return term->content.value_tree->value;
            } else
                return expr_node->token.type;   // expr_node->node_type == NT_LITERAL || NT_DATATYPE



        // func call
        case 1:
            if (expr_node->node_type == NT_ID) {
                term = bst_search(global->tree, key);
                if (term == NULL) {
                    bst_free_scope(global);
                    ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, expr_node, NULL);
                }

                if (term->content.value_tree == NULL)
                    sem_func_declare(sem_find_func(expr_node), global);

                return sem_func_eval(global, expr_node);

            } else    // expr_node->node_type == NT_BUILTIN
                return sem_eval_builtin(scope, expr_node->children[0]);



        case 2:
            left = sem_expr_type_eval(scope, expr_node->children[0]);
            right = sem_expr_type_eval(scope, expr_node->children[1]);

            if (expr_node->node_type == NT_BOOL_EXPR) {
                // Num > Num
                // Num >= Num
                // Num < Num
                // Num <= Num
                if (expr_node->token.type == TT_GT ||
                    expr_node->token.type == TT_GE ||
                    expr_node->token.type == TT_LT ||
                    expr_node->token.type == TT_LE) {
                    if ((left == TT_INT || left == TT_FLOAT || left == TT_RUNNING_NUM) &&
                        (right == TT_INT || right == TT_FLOAT || right == TT_RUNNING_NUM))
                        return TT_BOOL;

                    // Ar_expr is Datatype
                } else if (expr_node->token.type == TT_KEYWORD_IS) {
                    if ((left != TT_BOOL) &&
                        (right == TT_KEYWORD_NUM ||
                         right == TT_KEYWORD_STRING ||
                         right == TT_KEYWORD_Null))
                        return TT_BOOL;

                    // Datatype == Datatype
                    // Datatype != Datatype
                } else if (left != TT_BOOL && right != TT_BOOL)
                    return TT_BOOL;

            } else {
                // Num + Num ||
                // Num - Num ||
                // Num * Num ||
                // Num / Num
                if ((left == TT_INT || left == TT_FLOAT || left == TT_RUNNING_NUM) &&
                    (right == TT_INT || right == TT_FLOAT || right == TT_RUNNING_NUM))
                    return TT_FLOAT;

                // String + String
                if ((left == TT_STRING || left == TT_RUNNING_STRING) &&
                    expr_node->token.type == TT_PLUS &&
                    (right == TT_STRING || right == TT_RUNNING_STRING))
                    return TT_STRING;

                // String * Num
                if ((left == TT_STRING || left == TT_RUNNING_STRING) &&
                    expr_node->token.type == TT_MUL &&
                    (right == TT_INT || right == TT_RUNNING_NUM))
                    return TT_STRING;
            }

            bst_free_scope(global);
            ast_error(ERROR_SEM_STATIC_COMPATIBILITY, MSG_SEM_INCOMPATIBLE, expr_node, NULL);

            break;


    }
}

ast_node_ptr sem_find_func(ast_node_ptr func_id) {
    ast_node_ptr root = func_id;

    while (root->parent != NULL)
        root = root->parent;

    for (int i = 0; i < root->n_of_children; i++) {
        if (strcmp(func_id->token.lexeme, root->children[i]->token.lexeme) == 0 &&
            // if func call id == func definition id
            func_id->children[0]->n_of_children ==
            root->children[i]->children[0]->n_of_children)   // if func call num of params == func definition num of params
            return root->children[i];
    }

    destroy_ast(func_id);
    return NULL;
}

token_type_t sem_func_eval(bst_scope_ptr global, ast_node_ptr func_node) {
    return TT_ERROR;
}





// evaluates correctness of parameters and returns return datatype of builtin function
token_type_t sem_eval_builtin(bst_scope_ptr scope, ast_node_ptr builtin_node) {
    unsigned int key;
    ast_node_ptr param_node = builtin_node->children[0];
    bst_node_ptr arg = NULL;
    bst_scope_ptr starting_scope = scope;


    if (strcmp(builtin_node->token.lexeme, "read_str") == 0) return TT_RUNNING_STRING;
    if (strcmp(builtin_node->token.lexeme, "read_num") == 0) return TT_RUNNING_NUM;

    if (strcmp(builtin_node->token.lexeme, "write") == 0 ||
        strcmp(builtin_node->token.lexeme, "str") == 0) {
        if (param_node->n_of_children != 1) {
            bst_destroy_symbol_table(scope);
            ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
        }

        if (param_node->children[0]->node_type != NT_LITERAL) {
            key = get_hash(param_node->children[0]);
            if (key == 0) {
                bst_destroy_symbol_table(scope);
                ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, builtin_node, NULL);
            }

            arg = bst_search(scope->tree, key);
            while (arg == NULL) {
                if (scope->parent == NULL) {
                    if (strncmp(param_node->children[0]->token.lexeme, "__", 2)) {
                        bst_destroy_symbol_table(scope);
                        ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, builtin_node, NULL);
                    } else if (strcmp(builtin_node->token.lexeme, "write") == 0) return TT_NULL;     // write(term)
                    else return TT_STRING;   // str(term)
                }

                scope = scope->parent;
                arg = bst_search(scope->tree, key);
            }
        }

        if (strcmp(builtin_node->token.lexeme, "write") == 0) return TT_NULL;    // write(term)
        else return TT_STRING;  // str(term)
    }

    if (strcmp(builtin_node->token.lexeme, "floor") == 0) {
        if (param_node->n_of_children != 1) {
            bst_destroy_symbol_table(scope);
            ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
        }

        if (param_node->children[0]->node_type != NT_LITERAL) {
            key = get_hash(param_node->children[0]);
            if (key == 0) {
                bst_destroy_symbol_table(scope);
                ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, builtin_node, NULL);
            }

            arg = bst_search(scope->tree, key);
            while (arg == NULL) {
                if (scope->parent == NULL) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, builtin_node, NULL);
                }

                scope = scope->parent;
                arg = bst_search(scope->tree, key);
            }

            if (arg->content.type != TT_FLOAT) {
                bst_destroy_symbol_table(scope);
                ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
            }
        } else if (param_node->children[0]->token.type != TT_FLOAT) {
            bst_destroy_symbol_table(scope);
            ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
        }

        return TT_INT;
    }

    if (strcmp(builtin_node->token.lexeme, "length") == 0) {
        if (param_node->n_of_children != 1) {
            bst_destroy_symbol_table(scope);
            ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
        }

        if (param_node->children[0]->node_type != NT_LITERAL) {
            key = get_hash(param_node->children[0]);
            if (key == 0) {
                bst_destroy_symbol_table(scope);
                ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, builtin_node, NULL);
            }

            arg = bst_search(scope->tree, key);
            while (arg == NULL) {
                if (scope->parent == NULL) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, builtin_node, NULL);
                }

                scope = scope->parent;
                arg = bst_search(scope->tree, key);
            }

            if (arg->content.type != TT_STRING) {
                bst_destroy_symbol_table(scope);
                ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
            }
        } else if (param_node->children[0]->token.type != TT_STRING) {
            bst_destroy_symbol_table(scope);
            ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
        }

        return TT_INT;
    }

    if (strcmp(builtin_node->token.lexeme, "substring") == 0) {
        if (param_node->n_of_children != 3) {
            bst_destroy_symbol_table(scope);
            ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
        }

        for (int i = 0; i < param_node->n_of_children; i++) {
            if (param_node->children[i]->node_type != NT_LITERAL) {
                key = get_hash(param_node->children[i]);
                if (key == 0) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, builtin_node, NULL);
                }

                while (arg == NULL) {
                    if (scope->parent == NULL) {
                        bst_destroy_symbol_table(scope);
                        ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, builtin_node, NULL);
                    }

                    scope = scope->parent;
                    arg = bst_search(scope->tree, key);
                }

                if (i) {
                    if (arg->content.type != TT_INT) {
                        bst_destroy_symbol_table(scope);
                        ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
                    }
                } else if (arg->content.type != TT_STRING) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
                }
            } else if (i) {
                if (param_node->children[i]->token.type != TT_INT) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
                }
            } else if (param_node->children[i]->token.type != TT_STRING) {
                bst_destroy_symbol_table(scope);
                ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
            }

            scope = starting_scope;
        }

        return TT_RUNNING_STRING;
    }

    if (strcmp(builtin_node->token.lexeme, "strcmp") == 0) {
        if (param_node->n_of_children != 2) {
            bst_destroy_symbol_table(scope);
            ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
        }

        for (int i = 0; i < param_node->n_of_children; i++) {
            if (param_node->children[i]->node_type != NT_LITERAL) {
                key = get_hash(param_node->children[i]);
                if (key == 0) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, builtin_node, NULL);
                }

                while (arg == NULL) {
                    if (scope->parent == NULL) {
                        bst_destroy_symbol_table(scope);
                        ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, builtin_node, NULL);
                    }

                    scope = scope->parent;
                    arg = bst_search(scope->tree, key);
                }

                if (arg->content.type != TT_STRING) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
                }
            } else if (param_node->children[i]->token.type != TT_STRING) {
                bst_destroy_symbol_table(scope);
                ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
            }

            scope = starting_scope;
        }

        return TT_INT;
    }

    if (strcmp(builtin_node->token.lexeme, "ord") == 0) {
        if (param_node->n_of_children != 2) {
            bst_destroy_symbol_table(scope);
            ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
        }

        for (int i = 0; i < param_node->n_of_children; i++) {
            if (param_node->children[i]->node_type != NT_LITERAL) {
                key = get_hash(param_node->children[i]);
                if (key == 0) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, builtin_node, NULL);
                }

                while (arg == NULL) {
                    if (scope->parent == NULL) {
                        bst_destroy_symbol_table(scope);
                        ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, builtin_node, NULL);
                    }

                    scope = scope->parent;
                    arg = bst_search(scope->tree, key);
                }

                if (i) {
                    if (arg->content.type != TT_INT) {
                        bst_destroy_symbol_table(scope);
                        ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
                    }
                } else if (arg->content.type != TT_STRING) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
                }
            } else if (i) {
                if (param_node->children[i]->token.type != TT_INT) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
                }
            } else if (param_node->children[i]->token.type != TT_STRING) {
                bst_destroy_symbol_table(scope);
                ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
            }

            scope = starting_scope;
        }

        return TT_INT;
    }

    if (strcmp(builtin_node->token.lexeme, "chr") == 0) {
        if (param_node->n_of_children != 1) {
            bst_destroy_symbol_table(scope);
            ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
        }

        if (param_node->children[0]->node_type != NT_LITERAL) {
            key = get_hash(param_node->children[0]);
            if (key == 0) {
                bst_destroy_symbol_table(scope);
                ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, builtin_node, NULL);
            }

            while (arg == NULL) {
                if (scope->parent == NULL) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, builtin_node, NULL);
                }

                scope = scope->parent;
                arg = bst_search(scope->tree, key);
            }

            if (arg->content.type != TT_INT) {
                bst_destroy_symbol_table(scope);
                ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
            }

        } else if (param_node->children[0]->token.type != TT_INT) {
            bst_destroy_symbol_table(scope);
            ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
        }

        return TT_STRING;
    }
}