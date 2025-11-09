// Implementace prekladace imperativniho jazyka IFJ25
// AST by Jan Špaček "xspacej00" on 10/18/2025


#include "symtable.h"



//Funciton to declare a variable in the current scope
int bst_declare_variable(bst_scope_ptr scope, char *name){
  if (scope == NULL) return;
  if (bst_search_scope(scope, get_hash(name)).type != TT_ERROR){
    //Variable already declared in this or parent scope
    return 1;
  } else{
    bst_node_content_t new_var = node_content_init(name, TT_ERROR);
    bst_insert(&scope->tree, get_hash(name), new_var);
    return 0;
  }
}

//Main function to define a variable type
int bst_define_variable(bst_scope_ptr scope, char *name, token_type_t type){
  bst_node_ptr node = bst_node_scope_search_ptr(scope, get_hash(name));
  if (node != NULL){
    node->content.type = type;
    return 0;
  } else{
    //Variable not declared in this or parent scope
    return 1;
  } 
}


//Support function to search through scopes of the BST
bst_node_content_t bst_search_scope(bst_scope_ptr scope, unsigned int key){
    bst_node_content_t temp = bst_search(scope->tree, key);
    if (temp.type == TT_ERROR){
        return bst_search_scope(scope->parent, key);
    }
    return temp;
}

//Support function to increase the scope of the BST
void bst_increase_scope(bst_scope_ptr *scope){
  bst_scope_ptr new_scope = malloc(sizeof(bst_scope_t));
  new_scope->tree = NULL;
  new_scope->parent = (*scope);
  new_scope->child = NULL;
  new_scope->n_of_children = 0;

  if((*scope) != NULL){
    (*scope)->n_of_children++;
    (*scope)->child = realloc((*scope)->child, sizeof(bst_scope_ptr) * (*scope)->n_of_children);
    (*scope)->child[(*scope)->n_of_children - 1] = new_scope;
  }

  (*scope) = new_scope;
}


//Support funtion to decrease the scope of the BST
void bst_decrease_scope(bst_scope_ptr *scope){
  (*scope) = (*scope)->parent;
}


//Support function to create empty node content
bst_node_content_t node_content_init(char *name, token_type_t type){
    bst_node_content_t node;
    node.name = name;
    node.type = type;
    node.args = NULL;
    node.n_of_arguments = -1;
    return node;
}


//To start freeing the symbol table from any scope depth
void bst_destroy_symbol_table(bst_scope_ptr scope){
    while (scope->parent != NULL)
    {
        scope = scope->parent;
    }
    bst_free_scope(scope);
}

//To completely free the symbol table
void bst_free_scope(bst_scope_ptr scope){
    for(int i = 0; i < scope->n_of_children; i++){
        bst_free_scope(scope->child[i]);
    }
    bst_dispose(&scope->tree);
    free(scope);
}

//To return an unsigned int key to the BST from a name
unsigned int get_hash(char *str) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash * 31) + *str++;
    }
    return hash;
}

//To insert an element into a BST
void bst_insert(bst_node_ptr *tree, unsigned int key, bst_node_content_t value){
  if ((*tree) == NULL) {

    (*tree) = malloc(sizeof(bst_node_t));
    if ((*tree) == NULL)
      return;
    (*tree)->key = key;
    (*tree)->content = value;
    (*tree)->left = NULL;
    (*tree)->right = NULL;
    return;
  }

  else if (key == (*tree)->key) {
    (*tree)->content = value;
    return;
  }


  else if ((*tree)->key > key) {
    bst_insert(&((*tree)->left), key, value);
    (*tree)->left = bst_balance((*tree)->left);
  }

  else if ((*tree)->key < key) {
    bst_insert(&((*tree)->right), key, value);
    (*tree)->right = bst_balance((*tree)->right);
  }
}

//For searching through a BST using a unique key with the return value of the success of the search. Can change the pointer inserted in the last argument to point to the found node's content
bst_node_content_t bst_search(bst_node_ptr tree, unsigned int key){
    if (tree == NULL) {
        bst_node_content_t error;
        error.type = TT_ERROR;
        return error;
    }else if (tree->key == key) {
        return tree->content;
    } else {
        if (tree->key > key) {
        return bst_search(tree->left, key);
        }
        else if (tree->key < key) {
        return bst_search(tree->right, key);
        }
    } 

    bst_node_content_t error;
    error.type = TT_ERROR;
    return error;
}


//For searching for a node pointer (not content) through a BST
bst_node_ptr bst_node_search_ptr(bst_node_ptr tree, unsigned int key){
    if (tree == NULL) {
        return NULL;
    }else if (tree->key == key) {
        return tree;
    } else {
        if (tree->key > key) {
        return bst_node_search_ptr(tree->left, key);
        }
        else if (tree->key < key) {
        return bst_node_search_ptr(tree->right, key);
        }
    } 

    return NULL;
}
bst_node_ptr bst_node_scope_search_ptr(bst_scope_ptr scope, unsigned int key){
    bst_node_ptr temp = bst_node_search_ptr(scope->tree, key);
    if (temp == NULL){
        return bst_node_scope_search_ptr(scope->parent, key);
    }
    return temp;
}



//To insert an element into a BST
void bst_insert(bst_node_ptr *tree, unsigned int key, bst_node_content_t value){
    if ((*tree) == NULL) {

        (*tree) = malloc(sizeof(bst_node_t));
        if ((*tree) == NULL)
        return;
        (*tree)->key = key;
        (*tree)->content = value;
        (*tree)->left = NULL;
        (*tree)->right = NULL;
        return;
    }

    else if (key == (*tree)->key) {
        (*tree)->content = value;
        return;
    }


    else if ((*tree)->key > key) {
        bst_insert(&((*tree)->left), key, value);
        (*tree)->left = bst_balance((*tree)->left);
    }

    else if ((*tree)->key < key) {
        bst_insert(&((*tree)->right), key, value);
        (*tree)->right = bst_balance((*tree)->right);
    }
}

//A helping function to replace a node by its rightmost child
void bst_replace_by_rightmost(bst_node_ptr target, bst_node_ptr *tree){
    if((*tree)->right != NULL) {
        bst_replace_by_rightmost(target, &(*tree)->right);
    }

    else {
        target->key = (*tree)->key;
        target->content = (*tree)->content;

        bst_node_ptr temp = *tree;
        *tree = (*tree)->left;
        free(temp);
    }
}

//To delete a specific node out of a BST using its key
void bst_delete(bst_node_ptr *tree, unsigned int key){
    if (*tree == NULL || tree == NULL){
        return;
        }

    if ((*tree)->key > key) {
        bst_delete(&((*tree)->left), key);
    }

    else if ((*tree)->key < key) {
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
        }

        else if ((*tree)->right == NULL && (*tree)->left != NULL) {
        bst_node_ptr temp = (*tree)->left;
        free((*tree)->content.args);
        free(*tree);
        *tree = temp;
        return;
        }

        else if ((*tree)->right != NULL && (*tree)->left != NULL) {
        bst_replace_by_rightmost(*tree, &(*tree)->left);
        return;
        }
    }
}

//To free and dispose of a whole BST
void bst_dispose(bst_node_ptr *tree)
{
  if ((*tree) == NULL) {
    return;
    }

  if ((*tree)->left != NULL)
    bst_dispose(&((*tree)->left));

  if ((*tree)->right != NULL)
    bst_dispose(&((*tree)->right));

  if ((*tree)->left == NULL && (*tree)->right == NULL){
    if((*tree)->content.args != NULL) free((*tree)->content.args);
    free((*tree)->content.name);
    free(*tree);
  }
  *tree = NULL;
}

//To return a pointer to a balanced BST based on the weight of its branches
bst_node_ptr bst_balance(bst_node_ptr tree){
  if(tree == NULL) return NULL;

  int balance = bst_weight(tree->left) - bst_weight(tree->right);
  if (balance > 1){
    tree = bst_rotate_r(tree);
  }else if( balance < -1){
    tree = bst_rotate_l(tree);
  }
  return tree;
}

//To return a pointer to a rotated BST node to the left in order to maintain balance
bst_node_ptr bst_rotate_l(bst_node_ptr node){
  bst_node_ptr temp = node->right;
  node->right = temp->left;
  temp->left = node;
  return temp;
}

//To return a pointer to a rotated BST node to the right in order to maintain balance
bst_node_ptr bst_rotate_r(bst_node_ptr node){
  bst_node_ptr temp = node->left;
  node->left = temp->right;
  temp->right = node;
  return temp;
}

//To measure the weight of a binary tree's children and return the biggest weight
int bst_weight(bst_node_ptr tree){
  if(tree == NULL) return 0;
  int left = bst_weight(tree->left);
  int right = bst_weight(tree->right);
  if(left > right){
    return left++;
  }else{
    return right++;
  }

}
