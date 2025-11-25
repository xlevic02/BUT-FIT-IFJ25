// Implementace prekladace imperativniho jazyka IFJ25
// AST by Jan Špaček "xspacej00" on 10/02/2025

#ifndef BUT_FIT_IFJ25_SYMTABLE_H
#define BUT_FIT_IFJ25_SYMTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"
#include "ast.h"


typedef struct bst_value_node bst_value_node_t;
typedef bst_value_node_t *bst_value_node_ptr;

struct bst_value_node {
    bst_value_node_ptr parent;
    bst_value_node_ptr *children;
    int n_of_children;
    int current_index;
    token_type_t value;
};


typedef struct bst_node_content_t {
    char *name;
    ast_node_type_t type;
    bst_value_node_ptr value_tree;
} bst_node_content_t;


typedef struct bst_node bst_node_t;
typedef bst_node_t *bst_node_ptr;

struct bst_node {
    unsigned int key;
    bst_node_content_t content;
    bst_node_ptr left;
    bst_node_ptr right;
};

typedef struct bst_scope bst_scope_t;
typedef bst_scope_t *bst_scope_ptr;

struct bst_scope {
    bst_node_ptr tree;
    bst_scope_ptr *children;
    bst_scope_ptr parent;
    int n_of_children;
    unsigned int key;
};

int bst_define_variable(bst_scope_ptr scope, ast_node_ptr ast_node);

int bst_increase_scope(bst_scope_ptr *scope);

void bst_decrease_scope(bst_node_ptr tree_node);

bst_node_content_t node_content_init(ast_node_ptr ast_node);

void bst_destroy_symbol_table(bst_scope_ptr scope);

void bst_free_scope(bst_scope_ptr scope);

unsigned int get_hash(ast_node_ptr ast_node);

int bst_insert(bst_node_ptr *tree, unsigned int key, bst_node_content_t value);

bst_node_ptr bst_search(bst_node_ptr tree, unsigned int key);

//bst_node_ptr bst_node_search_ptr(bst_node_ptr tree, unsigned int key);
//bst_node_ptr bst_node_scope_search_ptr(bst_scope_ptr scope, unsigned int key);
void bst_replace_by_rightmost(bst_node_ptr target, bst_node_ptr *tree);

void bst_delete(bst_node_ptr *tree, unsigned int key);

void bst_dispose(bst_node_ptr *tree);

bst_node_ptr bst_balance(bst_node_ptr tree);

bst_node_ptr bst_rotate_r(bst_node_ptr tree);

bst_node_ptr bst_rotate_l(bst_node_ptr tree);

int bst_weight(bst_node_ptr tree);

bool is_func(ast_node_ptr ast_node);

unsigned int small_hash(char* str);

int bst_increase_var_reach(bst_node_ptr scope_tree);

void bst_free_content(bst_value_node_ptr value_tree);

bst_node_ptr bst_declare_variable(bst_node_ptr var_def_node, token_type_t value);

#endif //BUT_FIT_IFJ25_SYMTABLE_H