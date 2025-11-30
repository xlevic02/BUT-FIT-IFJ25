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
                exit(ERROR_SEM_REDEF);
        }

        sem_func_declare(global, root->children[i]);

        if (strcmp(root->children[i]->token.lexeme, "main") == 0) {
            if (!found_main && main_index == -1) {
                main_index = i;
                found_main = true;
            } else {
                // if the num of params of the last possible main func IS NOT 0
                if (root->children[main_index]->children[0]->n_of_children != 0) {

                    // if the num of params of the new possible main func IS NOT 0
                    if (root->children[i]->children[0]->n_of_children != 0) {
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

    for(int i = 0; i < global->n_of_children; i++) {
        if(global->children[i]->key == get_hash(root->children[main_index])) {
            sem_block_eval(global->children[i], root->children[main_index]->children[1]);
            return global;
        }
    }
}


void sem_func_declare(bst_scope_ptr global, ast_node_ptr func_decl_node) {
    if(bst_increase_scope(&global)) {
        bst_destroy_symbol_table(global);
        ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, func_decl_node, NULL);
    }

    bst_scope_ptr func_scope = global;
    global = global->parent;

    func_scope->key = get_hash(func_decl_node);

    if(func_decl_node->n_of_children != 1)
        for(int i = 0; i < func_decl_node->children[0]->n_of_children; i++)
            if(bst_define_variable(func_scope, func_decl_node->children[0]->children[i])) {
                bst_destroy_symbol_table(func_scope);
                ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, func_decl_node, NULL);
            }
} //sem_func_declare





void sem_block_eval(bst_scope_ptr scope, ast_node_ptr block_node) {
    bst_scope_ptr old_scope = scope;
    bst_value_node_ptr new_node = NULL;
    ast_node_ptr func_decl_node = block_node;
    unsigned int key;
    token_type_t condition;



    for (int i = 0; i < block_node->n_of_children; i++) {
        switch (block_node->children[i]->node_type) {
            case NT_VAR_DEF:
                if (strncmp(block_node->children[i]->token.lexeme, "__", 2) == 0)
                    while (scope->parent != NULL)
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

                bst_node_ptr def_node = bst_search(scope->tree, key);
                while (def_node == NULL) {
                    if (scope->parent == NULL) {
                        if (strncmp(block_node->children[i]->children[0]->token.lexeme, "__", 2) != 0) {
                            ast_node_ptr root = block_node;
                            while(root->parent != NULL)
                                root = root->parent;

                            bool found_setter = false;
                            for(int j = 0; j < root->n_of_children; j++) {
                                if (strcmp(root->children[j]->token.lexeme,
                                           block_node->children[i]->children[0]->token.lexeme) == 0 &&
                                    root->children[j]->node_type == NT_SETTER) {
                                    key = get_hash(root->children[j]);
                                    if (key == 0) {
                                        bst_destroy_symbol_table(scope);
                                        ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, block_node, NULL);
                                    }

                                    def_node = bst_search(scope->tree, key);
                                    found_setter = true;
                                    scope = old_scope;
                                    break;
                                }
                            }

                            if(found_setter)
                                break;

                            bst_destroy_symbol_table(scope);
                            ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, block_node, NULL);
                        } else {
                            if (bst_define_variable(scope, block_node->children[i]->children[0])) {
                                bst_destroy_symbol_table(scope);
                                ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, block_node, NULL);
                            }

                            def_node = bst_search(scope->tree, key);

                            break;
                        }

                    } //if

                    scope = scope->parent;
                    def_node = bst_search(scope->tree, key);
                } //while

                scope = old_scope;

                token_type_t expr_type = sem_expr_type_eval(scope, block_node->children[i]->children[1]);

                if(expr_type == TT_BOOL) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_SEM_STATIC_COMPATIBILITY, MSG_SEM_BOOL_IN_VAR, block_node, NULL);
                }


                if (def_node->content.type == NT_SETTER) {
                    sem_func_eval(old_scope, block_node->children[i]->children[0], expr_type);

                    scope = old_scope;
                    break;
                }

                scope = old_scope;

                def_node = bst_declare_variable(def_node, expr_type);
                if (def_node == (bst_node_ptr) -1) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, block_node, NULL);
                }

                break;

            case NT_IF_STATEMENT:
                condition = sem_expr_type_eval(old_scope, block_node->children[i]->children[0]);

                if (condition != TT_NULL &&
                    condition != TT_FALSE) {
                    if (bst_increase_scope(&scope)) {
                        bst_destroy_symbol_table(scope);
                        ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, block_node, NULL);
                    }



                    sem_block_eval(scope, block_node->children[i]->children[1]);

                    while (scope->parent != NULL) {
                        scope = scope->parent;
                        bst_decrease_scope(scope->tree);
                    }

                    scope = old_scope;
                }


                // if ELSE exists
                if(block_node->children[i]->n_of_children > 2) {
                    if (bst_increase_scope(&scope)) {
                        bst_destroy_symbol_table(scope);
                        ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, block_node, NULL);
                    }


                    sem_block_eval(scope, block_node->children[i]->children[2]);
                    while(scope->parent != NULL) {
                        scope = scope->parent;
                        bst_decrease_scope(scope->tree);
                    }

                    scope = old_scope;
                }

                break;

            case NT_WHILE:
                condition = sem_expr_type_eval(scope, block_node->children[i]->children[0]);

                if (condition == TT_NULL ||
                    condition == TT_FALSE)
                    break;

                if (bst_increase_scope(&scope)) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, block_node, NULL);
                }

                sem_block_eval(scope, block_node->children[i]->children[1]);
                while (scope->parent != NULL) {
                    scope = scope->parent;
                    bst_decrease_scope(scope->tree);
                }

                scope = old_scope;

                break;

            case NT_BLOCK:
                if (bst_increase_scope(&scope)) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, block_node, NULL);
                }

                sem_block_eval(scope, block_node->children[i]->children[0]);
                while (scope->parent != NULL) {
                    scope = scope->parent;
                    bst_decrease_scope(scope->tree);
                }

                scope = old_scope;

                break;

            case NT_RETURN:
                while  (func_decl_node->node_type != NT_FUNC_DECL &&
                        func_decl_node->node_type != NT_GETTER &&
                        func_decl_node->node_type != NT_SETTER)
                    func_decl_node = func_decl_node->parent;

                while (scope->parent != NULL)
                    scope = scope->parent;

                key = get_hash(func_decl_node);
                if (key == 0) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, block_node, NULL);
                }

                bst_node_ptr func_node = bst_search(scope->tree, key);
                scope = old_scope;

                if (func_node->content.value_tree->value == TT_ERROR) {
                    new_node = func_node->content.value_tree;
                    new_node->children = malloc(sizeof(bst_value_node_ptr) * ++new_node->n_of_children);
                } else {
                    new_node = func_node->content.value_tree->parent;
                    new_node->children = realloc(func_node->content.value_tree->parent->children,
                                                 sizeof(bst_value_node_ptr) * ++new_node->n_of_children);
                }

                if (new_node->children == NULL) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_REALLOC, block_node, NULL);
                }



                new_node->children[new_node->n_of_children - 1] = malloc(sizeof(bst_value_node_t));
                if (new_node->children[new_node->n_of_children - 1] == NULL) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, block_node, NULL);
                }

                new_node->children[new_node->n_of_children - 1]->parent = new_node;
                new_node->children[new_node->n_of_children - 1]->children = NULL;
                new_node->children[new_node->n_of_children - 1]->current_index = -1;

                func_node->content.value_tree = new_node->children[new_node->n_of_children - 1];
                func_node->content.value_tree->parent->current_index =
                        func_node->content.value_tree->parent->n_of_children - 1;

                if (block_node->children[i]->n_of_children != 0) {
                    func_node->content.value_tree->value = sem_expr_type_eval(scope,
                                                                              block_node->children[i]->children[0]);

                    return;
                } else
                    func_node->content.value_tree->value = TT_NULL;

                return;

            default:
                break;
        } //switch

        scope = old_scope;
    } //for
}


token_type_t sem_expr_type_eval(bst_scope_ptr scope, ast_node_ptr expr_node) {
    token_type_t left;
    token_type_t right;
    token_type_t getter_value;
    bst_node_ptr term = NULL;
    bst_scope_ptr func_scope = NULL;
    bst_scope_ptr old_scope = scope;
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
        // var, getter or literal
        case 0:
            if (expr_node->node_type == NT_ID) {
                term = bst_search(scope->tree, key);
                while (term == NULL) {
                    if (scope->parent == NULL) {
                        if (strncmp(expr_node->token.lexeme, "__", 2) != 0) {
                            ast_node_ptr root = expr_node;
                            while(root->parent != NULL)
                                root = root->parent;

                            getter_value = check_getter(old_scope, root, expr_node);
                            if(getter_value != TT_ERROR)
                                return getter_value;



                            bst_free_scope(global);
                            ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, expr_node, NULL);
                        } else {
                            if(bst_define_variable(scope, expr_node)) {
                                bst_destroy_symbol_table(scope);
                                ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, expr_node, NULL);
                            }
                            return TT_NULL;
                        }
                    }

                    scope = scope->parent;
                    term = bst_search(scope->tree, key);
                }

                return term->content.value_tree->value;
            } else
                return expr_node->token.type;   // expr_node->node_type == NT_LITERAL || NT_DATATYPE



        // func call
        case 1:
            term = bst_search(global->tree, key);
            if (term == NULL) {
                bst_free_scope(global);
                ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, expr_node, NULL);
            }

            return sem_func_eval(scope, expr_node, TT_ERROR);


        case 2:
            // builtin
            if (expr_node->children[0]->node_type == NT_ID &&
                expr_node->children[1]->node_type == NT_PARAM) {
                return sem_eval_builtin(scope, expr_node);
            }



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
                    if ((left == TT_INT || left == TT_FLOAT || left == TT_RUNNING_NUM || left == TT_RUNNING_UNDEF) &&
                        (right == TT_INT || right == TT_FLOAT || right == TT_RUNNING_NUM || right == TT_RUNNING_UNDEF))
                        return TT_BOOL;

                    // Ar_expr is Datatype
                } else if (expr_node->token.type == TT_KEYWORD_IS) {
                    if ((left != TT_BOOL) &&
                        (right == TT_KEYWORD_NUM ||
                         right == TT_KEYWORD_STRING ||
                         right == TT_KEYWORD_Null)) {
                        switch(right) {
                            case TT_KEYWORD_NUM:
                                if (left == TT_INT ||
                                    left == TT_FLOAT)
                                    return TT_TRUE;

                                if (left == TT_RUNNING_NUM ||
                                    left == TT_RUNNING_UNDEF)
                                    return TT_BOOL;

                                return TT_FALSE;

                            case TT_KEYWORD_STRING:
                                if(left == TT_STRING)
                                    return TT_TRUE;

                                if (left == TT_RUNNING_STRING ||
                                    left == TT_RUNNING_UNDEF)
                                    return TT_BOOL;

                                return TT_FALSE;

                            case TT_KEYWORD_Null:
                                if(left == TT_NULL)
                                    return TT_TRUE;

                                if (left == TT_RUNNING_NUM ||
                                    left == TT_RUNNING_STRING ||
                                    left == TT_RUNNING_UNDEF)
                                    return TT_BOOL;

                                return TT_FALSE;
                        }
                    }

                    // Datatype == Datatype
                    // Datatype != Datatype
                } else if (left != TT_BOOL && right != TT_BOOL) {

                    switch(left) {
                        case TT_NULL:
                            if(right == TT_NULL)
                                return TT_TRUE;
                            else if(right == TT_RUNNING_NUM ||
                                    right == TT_RUNNING_STRING ||
                                    right == TT_RUNNING_UNDEF)
                                return TT_BOOL;
                            else
                                return TT_FALSE;

                        case TT_INT:
                        case TT_FLOAT:
                            if (right == TT_INT ||
                                right == TT_FLOAT ||
                                right == TT_RUNNING_NUM ||
                                right == TT_RUNNING_UNDEF)
                                return TT_BOOL;

                            return TT_FALSE;

                        case TT_STRING:
                            if (right == TT_STRING ||
                                right == TT_RUNNING_STRING ||
                                right == TT_RUNNING_UNDEF)
                                return TT_BOOL;

                            return TT_FALSE;

                        case TT_RUNNING_NUM:
                            if (right == TT_NULL ||
                                right == TT_INT ||
                                right == TT_FLOAT ||
                                right == TT_RUNNING_NUM ||
                                right == TT_RUNNING_UNDEF)
                                return TT_BOOL;

                            return TT_FALSE;

                        case TT_RUNNING_STRING:
                            if (right == TT_NULL ||
                                right == TT_STRING ||
                                right == TT_RUNNING_STRING ||
                                right == TT_RUNNING_UNDEF)
                                return TT_BOOL;

                            return TT_FALSE;

                        case TT_RUNNING_UNDEF:
                            return TT_BOOL;
                    } //switch(left)
                } //if


            } else {
                // Num + Num ||
                // Num - Num ||
                // Num * Num ||
                // Num / Num
                if ((left == TT_INT || left == TT_FLOAT || left == TT_RUNNING_NUM || left == TT_RUNNING_UNDEF) &&
                    (right == TT_INT || right == TT_FLOAT || right == TT_RUNNING_NUM || right == TT_RUNNING_UNDEF))
                    return TT_RUNNING_UNDEF;

                // String + String
                if ((left == TT_STRING || left == TT_RUNNING_STRING || left == TT_RUNNING_UNDEF) &&
                    expr_node->token.type == TT_PLUS &&
                    (right == TT_STRING || right == TT_RUNNING_STRING || right == TT_RUNNING_UNDEF))
                    return TT_RUNNING_STRING;

                // String * Num
                if ((left == TT_STRING || left == TT_RUNNING_STRING || left == TT_RUNNING_UNDEF) &&
                    expr_node->token.type == TT_MUL &&
                    (right == TT_INT || right == TT_RUNNING_NUM || right == TT_RUNNING_UNDEF))
                    return TT_RUNNING_STRING;
            } //if



            bst_free_scope(global);
            ast_error(ERROR_SEM_STATIC_COMPATIBILITY, MSG_SEM_INCOMPATIBLE, expr_node, NULL);

            break;

    } //switch
}

ast_node_ptr sem_find_func(ast_node_ptr func_id, ast_node_type_t looking_for) {
    ast_node_ptr root = func_id;

    while (root->parent != NULL)
        root = root->parent;

    for (int i = 0; i < root->n_of_children; i++) {
        if (strcmp(func_id->token.lexeme, root->children[i]->token.lexeme)) // if func call id != func definition id
            continue;

        if (func_id->n_of_children != 0) {
            if (func_id->children[0]->n_of_children ==
                root->children[i]->children[0]->n_of_children)   // if func call num of params == func definition num of params
                return root->children[i];
        } else if (root->children[i]->node_type == looking_for)
            return root->children[i];
    }

    return NULL;
}


token_type_t sem_func_eval(bst_scope_ptr param_origin_scope, ast_node_ptr func_call_node, token_type_t setter_param) {
    unsigned int key;
    int i = 0;
    int n_of_children = 0;
    ast_node_type_t looking_for = NT_FUNC_DECL;
    ast_node_ptr root = func_call_node;
    bst_node_ptr param_node;    // variable that was sent
    bst_node_ptr arg_node;      // local variable with received value
    bst_node_ptr func_def_node;
    bst_scope_ptr old_scope = param_origin_scope;
    bst_scope_ptr func_scope = param_origin_scope;
    while (func_scope->parent != NULL)
        func_scope = func_scope->parent;

    func_scope = func_scope->children[0];

    if(func_call_node->n_of_children != 0) {
        key = get_hash(func_call_node);
        if(key == 0) {
            bst_destroy_symbol_table(func_scope);
            ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, func_call_node, NULL);
        }

        while(root->parent != NULL)
            root = root->parent;



        for(i = 1; i <= func_scope->parent->n_of_children; i++) {
            if(func_scope->key == key)
                break;


            if (i == func_scope->parent->n_of_children) {
                bst_destroy_symbol_table(func_scope);
                ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, func_call_node, NULL);
            } else
                func_scope = func_scope->parent->children[i];
        }


        key = get_hash(func_call_node);
        if (key == 0) {
            bst_destroy_symbol_table(func_scope);
            ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, func_call_node, NULL);
        }

        func_def_node = bst_search(func_scope->parent->tree, key);



        if (func_def_node->content.value_tree->value != TT_ERROR)
            n_of_children = func_def_node->content.value_tree->parent->n_of_children;


        for (i = 0; i < func_call_node->children[0]->n_of_children; i++) {
            key = get_hash(func_call_node->children[0]->children[i]);
            if (key == 0) {
                bst_destroy_symbol_table(func_scope);
                ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, func_call_node, NULL);
            }

            param_node = bst_search(param_origin_scope->tree, key);
            while (param_node == NULL) {
                if (param_origin_scope->parent == NULL) {
                    bst_destroy_symbol_table(func_scope);
                    ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, func_call_node, NULL);
                }

                param_origin_scope = param_origin_scope->parent;
                param_node = bst_search(param_origin_scope->tree, key);
            }

            param_origin_scope = old_scope;


            key = get_hash(sem_find_func(func_call_node, NT_FUNC_DECL)->children[0]->children[i]);
            if (key == 0) {
                bst_destroy_symbol_table(func_scope);
                ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, func_call_node, NULL);
            }

            arg_node = bst_search(func_scope->tree, key);

            arg_node = bst_declare_variable(arg_node, param_node->content.value_tree->value);

            if (arg_node == (bst_node_ptr) -1) {
                bst_destroy_symbol_table(func_scope);
                ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, func_call_node, NULL);
            }
        }
    } else {
        if(setter_param == TT_ERROR)
            looking_for = NT_GETTER;
        else
            looking_for = NT_SETTER;

        key = get_hash(sem_find_func(func_call_node, looking_for));



        for(i = 1; i <= func_scope->parent->n_of_children; i++) {
            if(func_scope->key == key)
                break;


            if (i == func_scope->parent->n_of_children) {
                bst_destroy_symbol_table(func_scope);
                ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, func_call_node, NULL);
            } else
                func_scope = func_scope->parent->children[i];
        }



        if (looking_for == NT_SETTER) { // func_call_node == NT_SETTER

            key = get_hash(sem_find_func(func_call_node, looking_for)->children[0]->children[0]);
            arg_node = bst_search(func_scope->tree, key);

            arg_node = bst_declare_variable(arg_node, setter_param);
            if (arg_node == (bst_node_ptr) -1) {
                bst_destroy_symbol_table(func_scope);
                ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, func_call_node, NULL);
            }

            sem_block_eval(func_scope, sem_find_func(func_call_node, looking_for)->children[1]);

            return TT_NULL;
        } else {
            key = get_hash(sem_find_func(func_call_node, looking_for));
            if (key == 0) {
                bst_destroy_symbol_table(func_scope);
                ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, func_call_node, NULL);
            }

            func_def_node = bst_search(func_scope->parent->tree, key);

            if (func_def_node->content.value_tree->value != TT_ERROR)
                n_of_children = func_def_node->content.value_tree->parent->n_of_children;
        }
    } //if

    ast_node_ptr func_decl_node = sem_find_func(func_call_node, looking_for);

    if(looking_for == NT_GETTER)
        sem_block_eval(func_scope, func_decl_node->children[0]);
    else
        sem_block_eval(func_scope, func_decl_node->children[1]);

    key = get_hash(func_decl_node);
    if(key == 0) {
        bst_destroy_symbol_table(func_scope);
        ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, func_decl_node, NULL);
    }


    bst_value_node_ptr func_value_tree = bst_search(func_scope->parent->tree, key)->content.value_tree;
    if(func_value_tree->value != TT_ERROR){
        bst_value_node_ptr first_child = func_value_tree;
        bst_value_node_ptr current_child;
        if(func_value_tree->children != NULL) {
            current_child = func_value_tree->children[0];

            while(1) {
                switch(first_child->value) {
                    case TT_INT:
                    case TT_FLOAT:
                        if(first_child->value == current_child->value)
                            break;

                        if((first_child->value == TT_INT || first_child->value == TT_FLOAT) &&
                           current_child->value == TT_INT || current_child->value == TT_FLOAT)
                            first_child->value = TT_FLOAT;
                        else if (current_child->value == TT_RUNNING_NUM)
                            first_child->value = TT_RUNNING_NUM;
                        else if(current_child->value != TT_INT &&
                                current_child->value != TT_FLOAT)
                            first_child->value = TT_RUNNING_UNDEF;

                        break;

                    case TT_STRING:
                        if(current_child->value == TT_STRING)
                            break;

                        if(current_child->value == TT_RUNNING_STRING)
                            first_child->value = TT_RUNNING_STRING;
                        else
                            first_child->value = TT_RUNNING_UNDEF;

                        break;

                    case TT_NULL:
                        if(current_child->value == TT_NULL)
                            break;

                        if (current_child->value == TT_INT ||
                            current_child->value == TT_FLOAT ||
                            current_child->value == TT_RUNNING_NUM)
                            first_child->value = TT_RUNNING_NUM;

                        if (current_child->value == TT_STRING ||
                            current_child->value == TT_RUNNING_STRING)
                            first_child->value = TT_RUNNING_STRING;
                        else
                            first_child->value = TT_RUNNING_UNDEF;

                        break;

                    case TT_RUNNING_NUM:
                        if (current_child->value == TT_STRING ||
                            current_child->value == TT_RUNNING_STRING ||
                            current_child->value == TT_RUNNING_UNDEF)
                            first_child->value = TT_RUNNING_UNDEF;

                        break;

                    case TT_RUNNING_STRING:
                        if (current_child->value != TT_STRING &&
                            current_child->value != TT_NULL &&
                            current_child->value != TT_RUNNING_STRING)
                            first_child->value = TT_RUNNING_UNDEF;

                        break;

                    default:
                        break;
                } //switch

                if(first_child->value == TT_RUNNING_UNDEF)
                    break;

                if(current_child->children != NULL)
                    current_child = current_child->children[0];
                else
                    break;
            } //while

            while(current_child->children != NULL)
                current_child = current_child->children[0];

            while(current_child != first_child) {
                current_child = current_child->parent;
                free(current_child->children[0]);
                free(current_child->children);
            }

            first_child->children = NULL;
        } //if
    }

    bst_decrease_scope(func_scope->parent->tree);

    return func_def_node->content.value_tree->value;

}


// evaluates correctness of parameters and returns return datatype of builtin function
token_type_t sem_eval_builtin(bst_scope_ptr scope, ast_node_ptr builtin_node) {
    unsigned int key;
    ast_node_ptr builtin_id = builtin_node->children[0];
    ast_node_ptr param_node = builtin_node->children[1];
    bst_node_ptr arg = NULL;
    bst_scope_ptr starting_scope = scope;
    token_type_t getter_value;
    token_type_t param_value;
    ast_node_ptr root = builtin_node;
    while(root->parent != NULL)
        root = root->parent;


    if (strcmp(builtin_id->token.lexeme, "read_str") == 0) return TT_RUNNING_STRING;
    if (strcmp(builtin_id->token.lexeme, "read_num") == 0) return TT_RUNNING_NUM;

    if (strcmp(builtin_id->token.lexeme, "write") == 0 ||
        strcmp(builtin_id->token.lexeme, "str") == 0) {
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
                        getter_value = check_getter(starting_scope, root, param_node->children[0]);
                        if(getter_value != TT_ERROR)
                            break;


                        bst_destroy_symbol_table(scope);
                        ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, builtin_node, NULL);
                    } else
                        bst_define_variable(scope, param_node->children[0]);


                    break;
                }

                scope = scope->parent;
                arg = bst_search(scope->tree, key);
            }
        }
                                                                //write : str
        return strcmp(builtin_id->token.lexeme, "write") == 0 ? TT_NULL : TT_STRING;
    } //if

    if (strcmp(builtin_id->token.lexeme, "floor") == 0) {
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
                    if(strncmp(param_node->children[0]->token.lexeme, "__", 2) == 0) {
                        bst_destroy_symbol_table(scope);
                        ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
                    }

                    getter_value = check_getter(starting_scope, root, param_node->children[0]);

                    if(getter_value != TT_ERROR)
                        break;

                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, builtin_node, NULL);
                }

                scope = scope->parent;
                arg = bst_search(scope->tree, key);
            }

            if(arg)
                param_value = arg->content.value_tree->value;
            else
                param_value = getter_value;

        } else
            param_value = param_node->children[0]->token.type;

        if (param_value != TT_FLOAT &&
            param_value != TT_RUNNING_NUM &&
            param_value != TT_RUNNING_UNDEF) {
            bst_destroy_symbol_table(scope);
            ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
        }

        return TT_INT;
    }

    if (strcmp(builtin_id->token.lexeme, "length") == 0) {
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
                    if(strncmp(param_node->children[0]->token.lexeme, "__", 2) == 0) {
                        bst_destroy_symbol_table(scope);
                        ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
                    }

                    getter_value = check_getter(starting_scope, root, param_node->children[0]);

                    if(getter_value != TT_ERROR)
                        break;

                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, builtin_node, NULL);
                }

                scope = scope->parent;
                arg = bst_search(scope->tree, key);
            }

            if(arg)
                param_value = arg->content.value_tree->value;
            else
                param_value = getter_value;
        } else
            param_value = param_node->children[0]->token.type;

        if (param_value != TT_STRING &&
            param_value != TT_RUNNING_STRING &&
            param_value != TT_RUNNING_UNDEF) {
            bst_destroy_symbol_table(scope);
            ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
        }

        return TT_INT;
    }

    if (strcmp(builtin_id->token.lexeme, "substring") == 0) {
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
                        if (strncmp(param_node->children[i]->token.lexeme, "__", 2) == 0) {
                            bst_destroy_symbol_table(scope);
                            ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node,
                                      NULL);
                        }

                        getter_value = check_getter(starting_scope, root, param_node->children[i]);
                        if (getter_value != TT_ERROR)
                            break;


                        bst_destroy_symbol_table(scope);
                        ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, builtin_node, NULL);
                    }

                    scope = scope->parent;
                    arg = bst_search(scope->tree, key);
                }

                if (arg)
                    param_value = arg->content.value_tree->value;
                else
                    param_value = getter_value;
            } else
                param_value = param_node->children[i]->token.type;


            if (i) {
                if (param_value != TT_INT &&
                    param_value != TT_RUNNING_NUM &&
                    param_value != TT_RUNNING_UNDEF) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
                }
            } else if  (param_value != TT_STRING &&
                        param_value != TT_RUNNING_STRING &&
                        param_value != TT_RUNNING_UNDEF) {
                bst_destroy_symbol_table(scope);
                ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
            }


            scope = starting_scope;
        }

        return TT_RUNNING_STRING;
    }

    if (strcmp(builtin_id->token.lexeme, "strcmp") == 0) {
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
                        if(strncmp(param_node->children[i]->token.lexeme, "__", 2) == 0) {
                            bst_destroy_symbol_table(scope);
                            ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
                        }

                        getter_value = check_getter(starting_scope, root, param_node->children[i]);

                        if(getter_value != TT_ERROR)
                            break;

                        bst_destroy_symbol_table(scope);
                        ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, builtin_node, NULL);
                    }

                    scope = scope->parent;
                    arg = bst_search(scope->tree, key);
                }

                if(arg)
                    param_value = arg->content.value_tree->value;
                else
                    param_value = getter_value;
            } else
                param_value = param_node->children[i]->token.type;

            if (param_value != TT_STRING &&
                param_value != TT_RUNNING_STRING &&
                param_value != TT_RUNNING_UNDEF) {
                bst_destroy_symbol_table(scope);
                ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
            }

            scope = starting_scope;
        } //for

        return TT_INT;
    }

    if (strcmp(builtin_id->token.lexeme, "ord") == 0) {
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
                        if(strncmp(param_node->children[i]->token.lexeme, "__", 2) == 0) {
                            bst_destroy_symbol_table(scope);
                            ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
                        }

                        getter_value = check_getter(starting_scope, root, param_node->children[i]);

                        if(getter_value != TT_ERROR)
                            break;

                        bst_destroy_symbol_table(scope);
                        ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, builtin_node, NULL);
                    }

                    scope = scope->parent;
                    arg = bst_search(scope->tree, key);
                }

                if(arg)
                    param_value = arg->content.value_tree->value;
                else
                    param_value = getter_value;
            } else
                param_value = param_node->children[i]->token.type;

            if (i) {
                if (param_value != TT_INT &&
                    param_value != TT_RUNNING_NUM &&
                    param_value != TT_RUNNING_UNDEF) {
                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
                }
            } else if  (param_value != TT_STRING &&
                        param_value != TT_RUNNING_STRING &&
                        param_value != TT_RUNNING_UNDEF) {
                bst_destroy_symbol_table(scope);
                ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
            }

            scope = starting_scope;
        }

        return TT_INT;
    }

    if (strcmp(builtin_id->token.lexeme, "chr") == 0) {
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
                    if(strncmp(param_node->children[0]->token.lexeme, "__", 2) == 0) {
                        bst_destroy_symbol_table(scope);
                        ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
                    }

                    getter_value = check_getter(starting_scope, root, param_node->children[0]);

                    if(getter_value != TT_ERROR)
                        break;

                    bst_destroy_symbol_table(scope);
                    ast_error(ERROR_SEM_UNDEF, MSG_SEM_UNDEF, builtin_node, NULL);
                }

                scope = scope->parent;
                arg = bst_search(scope->tree, key);
            }

            if(arg)
                param_value = arg->content.value_tree->value;
            else
                param_value = getter_value;
        } else
            param_value = param_node->children[0]->token.type;

        if (param_value != TT_INT &&
            param_value != TT_RUNNING_NUM &&
            param_value != TT_RUNNING_UNDEF) {
            bst_destroy_symbol_table(scope);
            ast_error(ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE, MSG_SEM_PARAM_NUM_OR_TYPE, builtin_node, NULL);
        }

        return TT_STRING;
    }
}


token_type_t check_getter(bst_scope_ptr scope, ast_node_ptr root, ast_node_ptr getter_call) {
    unsigned int key;
    bst_scope_ptr global = scope;
    while(global->parent != NULL)
        global = global->parent;



    for(int i = 0; i < root->n_of_children; i++)
        if (strcmp(root->children[i]->token.lexeme, getter_call->token.lexeme) == 0 &&
            root->children[i]->node_type == NT_GETTER) {

            key = get_hash(root->children[i]);
            if(key == 0) {
                bst_destroy_symbol_table(scope);
                ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, root, NULL);
            }

            sem_func_eval(scope, getter_call, TT_ERROR);

            return bst_search(global->tree, key)->content.value_tree->value;
        }

    return TT_ERROR;
}