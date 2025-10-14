//implementace abstraktního syntaktického stromu
//AST by Jan Špaček <xspacej00> on 02/10/2025

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"

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
} ast_node_t;