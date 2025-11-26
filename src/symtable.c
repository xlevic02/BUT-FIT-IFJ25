// Implementace prekladace imperativniho jazyka IFJ25
// Symtable by Jan Špaček "xspacej00" on 10/18/2025
//             Jan Frantisek "xlevic02" Levicek on 11/18/2025


#include "symtable.h"


//Function to define a variable in the current scope
int bst_define_variable(bst_scope_ptr scope, ast_node_ptr ast_node) {
    bst_node_content_t content = node_content_init(ast_node);
    unsigned int key = get_hash(ast_node);
    if (key == 0)
        return ERROR_INTERNAL;


    if (scope->tree != NULL)
        if ((bst_search(scope->tree, key)) != NULL)
            return ERROR_SEM_REDEF;

//printf("sym def var malloc\n");
    content.value_tree = malloc(sizeof(bst_value_node_t));
    if (content.value_tree == NULL)
        return ERROR_INTERNAL;

    content.value_tree->parent = NULL;
    content.value_tree->children = NULL;
    content.value_tree->n_of_children = 0;
    content.value_tree->current_index = -1;


    if(content.type == NT_VAR_DEF || strncmp(content.name, "__", 2) == 0)
        content.value_tree->value = TT_NULL;
    else
        content.value_tree->value = TT_ERROR;


    if (bst_insert(&scope->tree, key, content)) {
        free(content.value_tree);
        return ERROR_INTERNAL;
    }
    //printf("outside of insert\n");

    return 0;
}


bst_node_ptr bst_declare_variable(bst_node_ptr var_def_node, token_type_t value) {
    //printf("decl var: node=%p, value_tree=%p, parent=%p, n_children=%d\n",
           (void*)var_def_node,
           (void*)var_def_node->content.value_tree,
           (void*)(var_def_node->content.value_tree ?
                   var_def_node->content.value_tree->parent : NULL),
           var_def_node->content.value_tree ?
           var_def_node->content.value_tree->n_of_children : -1);

    //TODO slop start
    if (var_def_node == NULL || var_def_node->content.value_tree == NULL)
        return (bst_node_ptr) -1;

    // Najdi kořenový node (nejvyšší parent == NULL)
    bst_value_node_ptr root = var_def_node->content.value_tree;
    while (root->parent != NULL)
        root = root->parent;

    bst_value_node_ptr agg = root;

    //printf("decl var symtable realloc\n");
    bst_value_node_ptr *new_children =
            realloc(agg->children, sizeof(bst_value_node_ptr) * (agg->n_of_children + 1));
    if (new_children == NULL)
        return (bst_node_ptr) -1;

    agg->children = new_children;

    //printf("sym decl var malloc\n");
    bst_value_node_ptr child = malloc(sizeof(bst_value_node_t));
    if (child == NULL)
        return (bst_node_ptr) -1;

    agg->children[agg->n_of_children] = child;
    child->parent = agg;
    child->children = NULL;
    child->n_of_children = 0;
    child->current_index = -1;
    child->value = value;

    agg->current_index = agg->n_of_children;
    agg->n_of_children++;

    // "aktuální hodnota" = právě vytvořený child
    var_def_node->content.value_tree = child;

    //printf("end of decl var\n");
    return var_def_node;


    /*
    bst_value_node_ptr new_value = var_def_node->content.value_tree->parent;

    printf("decl var symtable realloc\n");
    new_value->children = realloc(var_def_node->content.value_tree->parent->children,
                                 sizeof(bst_value_node_ptr) * ++new_value->n_of_children);
    if(new_value->children == NULL)
        return (bst_node_ptr) -1;


    printf("sym decl var malloc\n");
    new_value->children[new_value->n_of_children - 1] = malloc(sizeof(bst_value_node_t));
    if(new_value->children[new_value->n_of_children - 1] == NULL)
        return (bst_node_ptr) -1;

    new_value->children[new_value->n_of_children - 1]->parent = new_value;
    new_value->children[new_value->n_of_children - 1]->children = NULL;
    new_value->children[new_value->n_of_children - 1]->n_of_children = 0;
    new_value->children[new_value->n_of_children - 1]->current_index = -1;
    new_value->current_index = new_value->n_of_children - 1;

    var_def_node->content.value_tree = new_value->children[new_value->n_of_children - 1];
    var_def_node->content.value_tree->value = value;

    printf("end of decl var\n");
    return var_def_node;
     */
}


//Support function to increase the scope of the BST
int bst_increase_scope(bst_scope_ptr *scope) {
    //printf("sym incr scope malloc\n");
    bst_scope_ptr new_scope = malloc(sizeof(bst_scope_t));
    if (new_scope == NULL)
        return ERROR_INTERNAL;


    new_scope->tree = NULL;
    new_scope->parent = (*scope);
    new_scope->children = NULL;
    new_scope->n_of_children = 0;
    new_scope->key = 0;

    if ((*scope) != NULL) {
        (*scope)->n_of_children++;
        //printf("incr scope symtable realloc\n");
        (*scope)->children = realloc((*scope)->children, sizeof(bst_scope_ptr) * (*scope)->n_of_children);
        if ((*scope)->children == NULL) {
            free(new_scope);
            return ERROR_INTERNAL;
        }

        if(bst_increase_var_reach((*scope)->tree))
            return ERROR_INTERNAL;

        (*scope)->children[(*scope)->n_of_children - 1] = new_scope;


    } else
        (*scope) = new_scope;

    //printf("end of incr scope\n");
    return 0;
}

int bst_increase_var_reach(bst_node_ptr scope_tree) {
    if(scope_tree == NULL)
        return 0;

    if(bst_increase_var_reach(scope_tree->left))
        return ERROR_INTERNAL;

    if(bst_increase_var_reach(scope_tree->right))
        return ERROR_INTERNAL;

    //int n_of_children = ++scope_tree->content.value_tree->n_of_children;
    bst_value_node_ptr value_tree = scope_tree->content.value_tree;

    // TODO: slop start
    if (value_tree == NULL) {
        //fprintf(stderr, "ERROR: NULL value_tree at key %u\n", scope_tree->key);
        exit(1);
    }

    if (value_tree->n_of_children < 0 || value_tree->n_of_children > 1000000) {
        //fprintf(stderr, "ERROR: corrupted n_of_children=%d at key %u\n",
                value_tree->n_of_children, scope_tree->key);
        exit(1);
    }

    int old_n = value_tree->n_of_children;
    int n_of_children = old_n + 1;

    //printf("sym realloc\n");
    void *tmp = realloc(value_tree->children,
                        sizeof(bst_value_node_ptr) * n_of_children);
    if (!tmp) {
        //fprintf(stderr, "ERROR: realloc failed at key %u (n=%d)\n",
                scope_tree->key, n_of_children);
        exit(1);
    }
    value_tree->children = tmp;
    //printf("sym var reach malloc\n");
    bst_value_node_ptr child = malloc(sizeof(bst_value_node_t));
    if (!child) {
        //fprintf(stderr, "ERROR: malloc failed at key %u\n", scope_tree->key);
        exit(1);
    }

    value_tree->n_of_children = n_of_children;
    value_tree->children[n_of_children - 1] = child;

    child->parent = value_tree;
    child->children = NULL;
    child->n_of_children = 0;
    child->current_index = -1;
    child->value = value_tree->value;

    value_tree->current_index = n_of_children - 1;


    return 0;


    /*
    printf("incr var reach symtable realloc\n");
    scope_tree->content.value_tree->children = realloc(value_tree->children, sizeof(bst_value_node_ptr) * n_of_children);
    if(scope_tree->content.value_tree->children == NULL) {
        fprintf(stderr, "NULL value_tree at key %u\n", scope_tree->key);
        return ERROR_INTERNAL;
    }
printf("here1\n");
    value_tree = scope_tree->content.value_tree;
    value_tree->children[n_of_children - 1] = malloc(sizeof(bst_value_node_t));
    if(value_tree->children[n_of_children - 1] == NULL)
        return ERROR_INTERNAL;
    printf("here2\n");
    value_tree->children[n_of_children - 1]->parent = value_tree;
    value_tree->children[n_of_children - 1]->children = NULL;
    value_tree->children[n_of_children - 1]->n_of_children = 0;
    value_tree->children[n_of_children - 1]->current_index = -1;
    value_tree->children[n_of_children - 1]->value = value_tree->value;
    printf("here3\n");
    value_tree->current_index = n_of_children - 1;
    scope_tree->content.value_tree = value_tree->children[n_of_children - 1];

    printf("end of var reach\n");
    return 0;
     */
}


//Support function to decrease the scope of the BST
void bst_decrease_scope(bst_node_ptr tree_node) {
    if(tree_node == NULL)
        return;

    bst_decrease_scope(tree_node->left);
    bst_decrease_scope(tree_node->right);

    if (tree_node->content.value_tree != NULL &&
        tree_node->content.value_tree->parent != NULL)
        tree_node->content.value_tree = tree_node->content.value_tree->parent;
}


//Support function to create empty node content
bst_node_content_t node_content_init(ast_node_ptr ast_node) {
    bst_node_content_t node;
    node.name = ast_node->token.lexeme;
    node.type = ast_node->node_type;
    node.value_tree = NULL;
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
        bst_free_scope(scope->children[i]);
    }

    bst_dispose(&scope->tree);
    free(scope);
}

//To return an unsigned int key to the BST from ast_node
unsigned int get_hash(ast_node_ptr ast_node) {
    char *name = ast_node->token.lexeme;
    int arg_count;
    char *buffer = NULL;
    unsigned int hash = 0;
    int size;
    const char* format;

    if (is_func(ast_node) &&
        ast_node->node_type != NT_GETTER &&
        ast_node->node_type != NT_SETTER &&
        ast_node->node_type != NT_BUILTIN) {
        size = strlen(name) + 17; // strlen(func:%s:%d) + '\0'
        //printf("sym hash malloc\n");
        buffer = malloc(sizeof(char) * size);
        if (buffer == NULL)
            return 0;

        arg_count = ast_node->children[0]->n_of_children;

        // function key: func:name:paramcount
        snprintf(buffer, sizeof(char) * size, "func:%s:%d", name, arg_count);

    } else {
        if (ast_node->node_type == NT_BUILTIN) {
            size = strlen(name) + 9; //strlen(builtin:%s) + '\0'
            format = "builtin:%s";
        } else {
            if(ast_node->node_type == NT_SETTER) {
                size = strlen(name) + 8; //strlen(setter:%s) + '\0'
                format = "setter:%s";
            } else if(ast_node->node_type == NT_GETTER) {
                size = strlen(name) + 8; //strlen(getter:%s) + '\0'
                format = "getter:%s";
            } else {
                size = strlen(name) + 5; //strlen(var:%s) + '\0'
                format = "var:%s";
            }
        }

        //printf("sym hash malloc\n");
        buffer = malloc(sizeof(char) * size);
        if (buffer == NULL)
            return 0;

        snprintf(buffer, sizeof(char) * size, format, name);
    }


    char* tmp = buffer;
    //printf("end of hash\n");
    while (*buffer) {
        hash = (hash * 31) + *buffer;
        buffer++;
    }

//printf("get hash: %s\n", tmp);
    free(tmp);
    return hash;
}

unsigned int small_hash(char *str) {
    unsigned int key = 0;

    while (*str) {
        key = (key * 31) + *str;
        str++;
    }

    return key;
}


//To insert an element into a BST
int bst_insert(bst_node_ptr *tree, unsigned int key, bst_node_content_t value) {
    //printf("bst_insert: tree=%p, *tree=%p, key=%u, name=%p\n",
    //       (void*)tree, (void*)(*tree), key, (void*)value.name);
    if ((*tree) == NULL) {

        //printf("sym insert malloc\n");
        //printf("%s\n", value.name);
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
    } else if (key < (*tree)->key) {
        int rc = bst_insert(&((*tree)->left), key, value);
        if(rc != 0) return rc;
        (*tree)->left = bst_balance((*tree)->left);
        return 0;
    } else { // key > (*tree)->key
        int rc = bst_insert(&((*tree)->right), key, value);
        if(rc != 0) return rc;
        (*tree)->right = bst_balance((*tree)->right);
        return 0;
    }
}



//For searching through a BST using a unique key with the return value of the success of the search. Can change the pointer inserted in the last argument to point to the found node's content
bst_node_ptr bst_search(bst_node_ptr tree, unsigned int key) {
    if (tree == NULL) {
        return tree;
    } else if (tree->key == key) {
        return tree;
    } else {
        if (key < tree->key) {
            return bst_search(tree->left, key);
        } else { // key > tree->key
            return bst_search(tree->right, key);
        }
    }
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



//To free and dispose of a whole BST
void bst_dispose(bst_node_ptr *tree) {
    if ((*tree) == NULL) {
        return;
    }

    if ((*tree)->left != NULL)
        bst_dispose(&((*tree)->left));

    if ((*tree)->right != NULL)
        bst_dispose(&((*tree)->right));

    bst_value_node_ptr value_tree = (*tree)->content.value_tree;

    if(value_tree != NULL) {
        while(value_tree->parent != NULL)
            value_tree = value_tree->parent;

        bst_free_content(value_tree);
    }

    free(*tree);
    *tree = NULL;
}

void bst_free_content(bst_value_node_ptr value_tree) {
    if(value_tree == NULL) return;

    for(int i = 0; i < value_tree->n_of_children; i++)
        bst_free_content(value_tree->children[i]);

    free(value_tree->children);
    free(value_tree);
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
    if(node == NULL || node->right == NULL) return node;
    bst_node_ptr temp = node->right;
    node->right = temp->left;
    temp->left = node;
    return temp;
}

//To return a pointer to a rotated BST node to the right in order to maintain balance
bst_node_ptr bst_rotate_r(bst_node_ptr node) {
    if(node == NULL || node->left == NULL) return node;
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
    return (left > right ? left : right) + 1;
}

bool is_func(ast_node_ptr ast_node) {
    if (ast_node->n_of_children)
        return true;
    else
        return false;
}