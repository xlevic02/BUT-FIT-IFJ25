//implementace abstraktního syntaktického stromu
//AST by Jan Špaček <xspacej00> on 02/10/2025

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "scanner.h"
#include "error.h"

typedef struct ast_node_t ast_node_t;
typedef struct ast_node_t* ast_node_ptr;


//structure for an AST node
typedef struct ast_node_t{
    ast_node_ptr* children; //pointer to a field of children
    token_t token; //token of data
    ast_node_ptr parent; //pointer to parent node
    union
    {
        int int_value;
        float float_value;
        char* string_value;
    } value;
    int n_of_children; //number of children
} ast_node_t;



ast_node_ptr create_ast();
ast_node_ptr ast_expression_node(token_t *current_token, int min_precedence, ast_node_ptr parent_node);
ast_node_ptr parse_primary(token_t *current_token, ast_node_ptr parent_node);
ast_node_ptr ast_regular_node(ast_node_ptr current_node, ast_node_ptr parent, token_t *current_token, int n_of_children);
ast_node_ptr ast_create_node(token_t token, ast_node_ptr parent);
ast_node_ptr ast_parameter_node(token_t *current_token, ast_node_ptr parent_node);
int ast_get_precedence(token_type_t type);
void ast_increase_children(ast_node_ptr current_node, ast_node_ptr new_node);
void ast_skip_EOL(token_t *current_token);
int ast_handle_prologue(token_t *current_token);
void ast_error(int err_num, const char* err_message, ast_node_ptr node, token_t *current_token);
void destroy_ast(ast_node_ptr node);
void free_ast(ast_node_ptr node);