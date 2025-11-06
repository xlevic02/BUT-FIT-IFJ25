//implementace abstraktního syntaktického stromu
//AST by Jan Špaček <xspacej00> on 02/10/2025

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"




typedef struct bst_node_content_t {
  char *name;
  token_type_t type;
  int n_of_arguments;   
  token_type_t *args;
} bst_node_content_t;


typedef struct bst_node bst_node_t;
typedef bst_node_t *bst_node_ptr;

typedef struct bst_node {
  unsigned int key;
  bst_node_content_t content;
  bst_node_ptr left;
  bst_node_ptr right;
} bst_node_t;

typedef struct bst_scope bst_scope_t;
typedef bst_scope_t *bst_scope_ptr;

typedef struct bst_scope {
  bst_node_ptr tree;
  bst_scope_ptr *child;
  bst_scope_ptr parent;
  int n_of_children;
} bst_scope_t;

int bst_declare_variable(bst_scope_ptr scope, char *name);
int bst_define_variable(bst_scope_ptr scope, char *name, token_type_t type);
bst_node_content_t bst_search_scope(bst_scope_ptr scope, unsigned int key);
void bst_increase_scope(bst_scope_ptr *scope);
void bst_decrease_scope(bst_scope_ptr *scope);
bst_node_content_t node_content_init(char *name, token_type_t type);
void bst_destroy_symbol_table(bst_scope_ptr scope);
void bst_free_scope(bst_scope_ptr scope);
unsigned int get_hash(char *str);
void bst_insert(bst_node_ptr *tree, unsigned int key, bst_node_content_t value);
bst_node_content_t bst_search(bst_node_ptr tree, unsigned int key);
bst_node_ptr bst_node_search_ptr(bst_node_ptr tree, unsigned int key);
bst_node_ptr bst_node_scope_search_ptr(bst_scope_ptr scope, unsigned int key);
void bst_replace_by_rightmost(bst_node_ptr target, bst_node_ptr *tree);
void bst_delete(bst_node_ptr *tree, unsigned int key);
void bst_dispose(bst_node_ptr *tree);
bst_node_ptr bst_balance(bst_node_ptr tree);
bst_node_ptr bst_rotate_r(bst_node_ptr tree);
bst_node_ptr bst_rotate_l(bst_node_ptr tree);
int bst_weight(bst_node_ptr tree);
