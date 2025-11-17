// Implementace prekladace imperativniho jazyka IFJ25
// AST by Jan Špaček "xspacej00" on 10/18/2025


#include "symtable.h"


//TODO -- add IJF functions to the symbol table upon initialization


//Function to define a variable in the current scope
int bst_define_variable(bst_scope_ptr scope, ast_node_ptr ast_node) {
    bst_node_content_t content = node_content_init(ast_node);
    unsigned int key = get_hash(ast_node);
    if(key == 0)
        return ERROR_INTERNAL;


    if(content.type == NT_FUNC_DECL) {
        content.n_of_arguments = ast_node->n_of_children;
        content.args = malloc(sizeof(token_type_t) * content.n_of_arguments);
        if(content.args == NULL)
            return ERROR_INTERNAL;

        for(int i = 0; i < ast_node->n_of_children; i++)
            content.args[i] = ast_node->children[i]->token.type;
    } else {
        content.n_of_arguments = 1;
        content.args = malloc(sizeof(token_type_t));
        if(content.args == NULL)
            return ERROR_INTERNAL;

        content.args[0] = TT_NULL;
    }


    if(scope->tree != NULL)
        if((bst_search(scope->tree, key)) != NULL)
            return ERROR_SEM_REDEF;


    if(bst_insert(&scope->tree, key, content))
        return ERROR_INTERNAL;


    return 0;
}

int bst_define_global_variable(bst_scope_ptr scope, char *name) {
    bst_scope_ptr global_scope = scope;
    while (global_scope->parent != NULL) {
        global_scope = global_scope->parent;
    }
    if (bst_search_scope(global_scope, get_hash(name)).type != TT_ERROR) {
        //Variable already defined in this or parent scope
        return 1;
    } else {
        bst_node_content_t new_var = node_content_init(name, TT_ERROR);
        if (bst_insert(&global_scope->tree, get_hash(name), new_var) == ERROR_INTERNAL)
            return ERROR_INTERNAL;

        return 0;
    }
}

//Main function to declare a variable type
int bst_declare_variable(bst_scope_ptr scope, ast_node_ptr variable_node, ast_node_ptr expr_node) {
    unsigned int key = get_hash(variable_node);
    if(key == 0)
        return ERROR_INTERNAL;

    bst_node_ptr old_node = bst_search(scope->tree, key);
    if(old_node == NULL)
        return ERROR_SEM_UNDEF;


    old_node->content.args = realloc(old_node->content.args, sizeof(token_type_t) * ++old_node->content.n_of_arguments);
    if(old_node->content.args == NULL)
        return ERROR_INTERNAL;


    switch(expr_node->node_type) {
        case NT_BOOL_EXPR:
            return ERROR_SEM_OTHER;

        case NT_AR_EXPR:
    }

    old_node->content.args[old_node->content.n_of_arguments - 1] = bst_eval_expr(expr_node);


    return 0;
}

token_type_t bst_eval_expr(ast_node_ptr expr_node) {
    switch(expr_node->node_type) {
        case NT_BOOL_EXPR:
            break;

        NT_AR_EXPR:


    }
}


/*
//Support function to search through scopes of the BST
bst_node_content_t bst_search_scope(bst_scope_ptr scope, unsigned int key) {
    bst_node_content_t temp = node_content_init(NULL, TT_ERROR);

    if (scope == NULL)
        return temp;

    temp = bst_search(scope->tree, key);
    if (temp.type == TT_ERROR) {
        return bst_search_scope(scope->parent, key);
    }
    return temp;
}
*/

//Support function to increase the scope of the BST
int bst_increase_scope(bst_scope_ptr *scope) {
    bst_scope_ptr new_scope = malloc(sizeof(bst_scope_t));
    if (new_scope == NULL)
        return ERROR_INTERNAL;


    new_scope->tree = NULL;
    new_scope->parent = (*scope);
    new_scope->children = NULL;
    new_scope->n_of_children = 0;

    if ((*scope) != NULL) {
        (*scope)->n_of_children++;
        (*scope)->children = realloc((*scope)->children, sizeof(bst_scope_ptr) * (*scope)->n_of_children);
        if ((*scope)->children == NULL) {
            free(new_scope);
            return ERROR_INTERNAL;
        }

        (*scope)->children[(*scope)->n_of_children - 1] = new_scope;
    } else
        (*scope) = new_scope;

    return 0;
}


//Support function to decrease the scope of the BST
void bst_decrease_scope(bst_scope_ptr *scope) {
    (*scope) = (*scope)->parent;
}


//Support function to create empty node content
bst_node_content_t node_content_init(ast_node_ptr ast_node) {
    bst_node_content_t node;
    node.name = ast_node->token.lexeme;
    node.type = ast_node->node_type;
    node.args = NULL;
    node.n_of_arguments = -1;
    return node;
}


//To start freeing the symbol table from any scope depth
void bst_destroy_symbol_table(bst_scope_ptr scope) {
    while (scope->parent != NULL) {
        scope = scope->parent;
    }
    bst_free_scope(scope);
}

//To completely free the symbol table
void bst_free_scope(bst_scope_ptr scope) {
    for (int i = 0; i < scope->n_of_children; i++) {
        bst_free_scope(scope->child[i]);
    }
    bst_dispose(&scope->tree);
    free(scope);
}

//To return an unsigned int key to the BST from ast_node
//TODO check if correct
unsigned int get_hash(ast_node_ptr ast_node) {
    char* name = ast_node->token.lexeme;
    int arg_count = ast_node->children[0]->n_of_children;
    char* buffer = NULL;
    unsigned int hash = 0;
    int size;

    if (is_func(ast_node)) {
        size = strlen(name) + 17; // strlen(func:%s:%d) + '\0'
        buffer = malloc(sizeof(char) * size);
        if(buffer == NULL)
            return 0;

        // function key: func:name:paramcount
        snprintf(buffer, sizeof(char) * size, "func:%s:%d", name, arg_count);
    } else {
        size = strlen(name) + 4; //strlen(var:%s) + '\0'
        buffer = malloc(sizeof(char) * size);
        if(buffer == NULL)
            return 0;

        // variable key: var:name
        snprintf(buffer, sizeof(char) * size, "var:%s", name);
    }

    for(char* p; *p; p++)
        hash = (hash * 31) + *p;

    free(buffer);

    return hash;
}


//To insert an element into a BST
int bst_insert(bst_node_ptr *tree, unsigned int key, bst_node_content_t value) {
    if ((*tree) == NULL) {

        (*tree) = malloc(sizeof(bst_node_t));
        if ((*tree) == NULL)
            return ERROR_INTERNAL;

        (*tree)->key = key;
        (*tree)->content = value;
        (*tree)->left = NULL;
        (*tree)->right = NULL;
        return 0;
    } else if (key == (*tree)->key) {
        (*tree)->content = value;
        return 0;
    } else if ((*tree)->key < key) {
        bst_insert(&((*tree)->left), key, value);
        (*tree)->left = bst_balance((*tree)->left);
    } else if ((*tree)->key > key) {
        bst_insert(&((*tree)->right), key, value);
        (*tree)->right = bst_balance((*tree)->right);
    }
}

//For searching through a BST using a unique key with the return value of the success of the search. Can change the pointer inserted in the last argument to point to the found node's content
bst_node_ptr bst_search(bst_node_ptr tree, unsigned int key) {
    if (tree == NULL) {
        return tree;
    } else if (tree->key == key) {
        return tree;
    } else {
        if (tree->key < key) {
            return bst_search(tree->left, key);
        } else if (tree->key > key) {
            return bst_search(tree->right, key);
        }
    }
}


//For searching for a node pointer (not content) through a BST
bst_node_ptr bst_node_search_ptr(bst_node_ptr tree, unsigned int key) {
    if (tree == NULL) {
        return NULL;
    } else if (tree->key == key) {
        return tree;
    } else {
        if (tree->key > key) {
            return bst_node_search_ptr(tree->left, key);
        } else if (tree->key < key) {
            return bst_node_search_ptr(tree->right, key);
        }
    }

    return NULL;
}

bst_node_ptr bst_node_scope_search_ptr(bst_scope_ptr scope, unsigned int key) {
    bst_node_ptr temp = bst_node_search_ptr(scope->tree, key);
    if (temp == NULL) {
        return bst_node_scope_search_ptr(scope->parent, key);
    }
    return temp;
}


//A helping function to replace a node by its rightmost child
void bst_replace_by_rightmost(bst_node_ptr target, bst_node_ptr *tree) {
    if ((*tree)->right != NULL) {
        bst_replace_by_rightmost(target, &(*tree)->right);
    } else {
        target->key = (*tree)->key;
        target->content = (*tree)->content;

        bst_node_ptr temp = *tree;
        *tree = (*tree)->left;
        free(temp);
    }
}

//To delete a specific node out of a BST using its key
void bst_delete(bst_node_ptr *tree, unsigned int key) {
    if (*tree == NULL) {
        return;
    }

    if ((*tree)->key < key) {
        bst_delete(&((*tree)->left), key);
    } else if ((*tree)->key > key) {
        bst_delete(&((*tree)->right), key);
    }

    if ((*tree)->key == key) {

        if ((*tree)->left == NULL && (*tree)->right == NULL) {
            free((*tree)->content.args);
            free(*tree);
            *tree = NULL;
            return;
        }

        if ((*tree)->left == NULL && (*tree)->right != NULL) {
            bst_node_ptr temp = (*tree)->right;
            free((*tree)->content.args);
            free(*tree);
            *tree = temp;
            return;
        } else if ((*tree)->right == NULL && (*tree)->left != NULL) {
            bst_node_ptr temp = (*tree)->left;
            free((*tree)->content.args);
            free(*tree);
            *tree = temp;
            return;
        } else if ((*tree)->right != NULL && (*tree)->left != NULL) {
            bst_replace_by_rightmost(*tree, &(*tree)->left);
            return;
        }
    }
}

//To free and dispose of a whole BST
void bst_dispose(bst_node_ptr *tree) {
    if ((*tree) == NULL) {
        return;
    }

    if ((*tree)->left != NULL)
        bst_dispose(&((*tree)->left));

    if ((*tree)->right != NULL)
        bst_dispose(&((*tree)->right));

    if ((*tree)->left == NULL && (*tree)->right == NULL) {
        if ((*tree)->content.args != NULL) free((*tree)->content.args);
        free((*tree)->content.name);
        free(*tree);
    }
    *tree = NULL;
}

//To return a pointer to a balanced BST based on the weight of its branches
bst_node_ptr bst_balance(bst_node_ptr tree) {
    if (tree == NULL) return NULL;

    int balance = bst_weight(tree->left) - bst_weight(tree->right);
    if (balance > 1) {
        tree = bst_rotate_r(tree);
    } else if (balance < -1) {
        tree = bst_rotate_l(tree);
    }
    return tree;
}

//To return a pointer to a rotated BST node to the left in order to maintain balance
bst_node_ptr bst_rotate_l(bst_node_ptr node) {
    bst_node_ptr temp = node->right;
    node->right = temp->left;
    temp->left = node;
    return temp;
}

//To return a pointer to a rotated BST node to the right in order to maintain balance
bst_node_ptr bst_rotate_r(bst_node_ptr node) {
    bst_node_ptr temp = node->left;
    node->left = temp->right;
    temp->right = node;
    return temp;
}

//To measure the weight of a binary tree's children and return the biggest weight
int bst_weight(bst_node_ptr tree) {
    if (tree == NULL) return 0;
    int left = bst_weight(tree->left);
    int right = bst_weight(tree->right);
    if (left > right) {
        return left++;
    } else {
        return right++;
    }
}

bool is_func(ast_node_ptr ast_node) {
    if(ast_node->n_of_children)
        return true;
    else
        return false;
}