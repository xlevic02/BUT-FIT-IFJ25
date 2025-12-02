// Implementace prekladace imperativniho jazyka IFJ25
// Semantic analysis by Jan Frantisek "xlevic02" Levicek on 11/9/2025

#ifndef BUT_FIT_IFJ25_SEM_ANALYSIS_H
#define BUT_FIT_IFJ25_SEM_ANALYSIS_H

#include<stdio.h>
#include<stdlib.h>
#include "ast.h"
#include "symtable.h"

typedef enum {
    BT_READ_STR,    // read_str()                           -> String | Null
    BT_READ_NUM,    // read_num()                           -> Num | Null
    BT_WRITE,       // write(term)                          -> Null
    BT_FLOOR,       // floor(term : Num)                    -> Num
    BT_STR,         // str(term)                            -> String
    BT_LENGTH,      // length(s: String)                    -> Num
    BT_SUBSTRING,   // substring(s: String, i: Num, j: Num) -> String | Null
    BT_STRCMP,      // strcmp(s1: String, s2: String)       -> Num
    BT_ORD,         // ord(s: String, i: Num)               -> Num
    BT_CHR          // chr(i: Num)                          -> String
}builtin_type_t;

bst_scope_ptr sem_start_analysis(ast_node_ptr root);
void sem_func_declare(bst_scope_ptr global, ast_node_ptr func_decl_node);
void sem_block_eval(bst_scope_ptr scope, ast_node_ptr block_node);
token_type_t sem_expr_type_eval(bst_scope_ptr scope, ast_node_ptr expr_node);
ast_node_ptr sem_find_func(ast_node_ptr func_id, ast_node_type_t looking_for);
token_type_t sem_func_eval(bst_scope_ptr param_origin_scope, ast_node_ptr func_call_node, token_type_t setter_param);
token_type_t sem_eval_builtin(bst_scope_ptr scope, ast_node_ptr builtin_node);
token_type_t check_getter(bst_scope_ptr scope, ast_node_ptr root, ast_node_ptr getter_call);


#endif //BUT_FIT_IFJ25_SEM_ANALYSIS_H