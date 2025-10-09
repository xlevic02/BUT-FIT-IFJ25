//implementace abstraktního syntaktického stromu
//AST by Jan Špaček <xspacej00> on 02/10/2025

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"



typedef struct bst_node_arguments_t {
  token_type_t type;
  bool is_nullable;
} bst_node_arguments_t;

typedef struct bst_node_content_t {
  char *name;
  token_type_t type;
  //bool is_nullable;
  //bool is_constant;
  //bool sem_is_used;//used in sematics for error 9 (const / var)
  //bool sem_is_changed;//used in sematics for error 9 (var) 
  int n_of_arguments;   
  bst_node_arguments_t *args;
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
  bst_scope_ptr child;
  bst_scope_ptr parent;
  bst_scope_ptr next;
} bst_scope_t;