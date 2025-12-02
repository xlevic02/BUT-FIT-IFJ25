// Implementace prekladace imperativniho jazyka IFJ25
// AST by Jan Špaček "xspacej00" on 10/02/2025

#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "scanner.h"
#include "error.h"

typedef struct ast_node_t ast_node_t;
typedef struct ast_node_t* ast_node_ptr;


typedef enum {
    NT_ROOT,            // Root                  [0]
    NT_VAR_DEF,         // Variable definition   [1]
    NT_FUNC_DECL,       // Function declaration  [2]
    NT_GETTER,          // Getter declaration    [3]
    NT_SETTER,          // Setter declaration    [4]
    NT_PARAM,           // Parameters            [5]
    NT_ID,              // Identifier            [6]
    NT_ASSIGN,          // Variable declaration  [7]
    NT_IF_STATEMENT,    // If statement          [8]
    NT_IF_BODY,         // If body               [9]
    NT_ELSE_BODY,       // Else body             [10]
    NT_BOOL_EXPR,       // Boolean expression    [11]
    NT_AR_EXPR,         // Arithmetic expression [12]
    NT_LITERAL,         // Literal               [13]
    NT_DATATYPE,        // Datatype              [14]
    NT_BUILTIN,         // Builtin function      [15]
    NT_WHILE,           // While                 [16]
    NT_WHILE_BODY,      // While body            [17]
    NT_BLOCK,           // Block                 [18]
    NT_RETURN           // Return                [19]
}ast_node_type_t;

//structure for an AST node
struct ast_node_t{
    ast_node_ptr parent; //pointer to parent node
    ast_node_ptr* children; //pointer to a field of children
    ast_node_type_t node_type;
    token_t token; //token of data
    union
    {
        int int_value; //number of children/
        float float_value;
        char* string_value;
    } value;
    int n_of_children; //number of children
};



ast_node_ptr create_ast();
ast_node_ptr ast_expression_node(token_t *current_token, ast_node_ptr parent_node);
ast_node_ptr ast_expression_inner(token_t *current_token, int min_precedence, ast_node_ptr parent_node, ast_node_ptr *upper_pointer);
ast_node_ptr parse_primary(token_t *current_token, ast_node_ptr parent_node, ast_node_ptr *upper_pointer);
ast_node_ptr ast_regular_node(ast_node_ptr current_node, ast_node_ptr parent, token_t *current_token, int n_of_children);
ast_node_ptr ast_create_node(token_t token, ast_node_ptr parent, ast_node_type_t node_type);
ast_node_ptr ast_parameter_node(token_t *current_token, ast_node_ptr parent_node);
ast_node_ptr ast_ifj_function_call_node(token_t *current_token, ast_node_ptr current_node);
int ast_get_precedence(token_type_t type);
void ast_increase_children(ast_node_ptr current_node, ast_node_ptr new_node);
void ast_skip_EOL(token_t *current_token);
int ast_handle_prologue(token_t *current_token);
void ast_error(int err_num, const char* err_message, ast_node_ptr node, token_t *current_token);
void destroy_ast(ast_node_ptr node);
void free_ast(ast_node_ptr *node);
const char* token_type_to_string(token_type_t type);
void ast_print_token(token_t token); // Debugging function to print token information

#endif // AST_H