// Implementace prekladace imperativniho jazyka IFJ25
// Semantic analysis by Jan Frantisek "xlevic02" Levicek on 11/9/2025

#ifndef BUT_FIT_IFJ25_SEM_ANALYSIS_H
#define BUT_FIT_IFJ25_SEM_ANALYSIS_H

#include<stdio.h>
#include<stdlib.h>
#include "ast.h"
#include "symtable.h"

bst_scope_ptr sem_start_analysis(ast_node_ptr root);
void sem_func_eval(ast_node_ptr func, bst_scope_ptr parent_scope);

#endif //BUT_FIT_IFJ25_SEM_ANALYSIS_H