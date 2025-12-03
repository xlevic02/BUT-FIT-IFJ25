// Implementace generatoru vysledneho kodu IFJcode25
//generator.h by Marek "xbalism00" Bališ on 11/11/25

#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ast.h"
#include "symtable.h"
#include "scanner.h"
#include "error.h"

#define MAX_STACK_SIZE 100
typedef struct var_node
    {
        char *original_id;
        char *new_id; 
        struct var_node *next;
    } var_node;
typedef struct scope_stack 
    {
        var_node *arr[MAX_STACK_SIZE];
        int top;
    } scope_stack;

typedef struct id_list_item 
    {
        ast_node_ptr node;       
        char *generated_id;      
        struct id_list_item *next;
    } id_list_item;
typedef enum 
    {
        VARIABLE,
        LITERAL_INT,
        LITERAL_FLOAT,
        LITERAL_STRING,
        LITERAL_BOOL,
        LITERAL_NULL
    }SYM_TYPE;
typedef enum
    {
        ADD,
        SUB,
        MUL,
        DIV,
        IDIV
    }ARITHMETIC;

typedef enum
    {
        LT,
        GT,
        EQ
    }RELATION;

typedef enum
    {
        AND,
        OR,
        NOT
    }BOOLEAN;

typedef enum
    {
        INT2FLOAT,
        FLOAT2INT,
        INT2CHAR,
        STRI2INT,
        FLOAT2STR,
        INT2STR
    }CONVERSION;

//Recursive function to proccess nodes
void generate_node(ast_node_ptr node, bst_scope_ptr *current_scope);

//Main function for generating code
int generate_code(ast_node_ptr root, bst_scope_ptr symtable);

//Checks the frame type
bool global_check(const char* var_name);

//Recursive function to search and define all global variables
void generate_globals(bst_node_ptr symtable);

//Creates a unique label for if, else, while
void get_unique_label(char* buffer, const char* prefix);

//Function to format string for IFJcode25
char* format_string_for_ifjcode(const char* lexeme);

//Function to format float for IFJcode25
char* format_float_for_ifjcode(const char* lexeme);

//Function to check if the node is a regular function, getter or setter
int get_function_type(char* func_name, int parameter_count);

//Shadow stack functions
void stack_init();
void stack_push(ast_node_ptr node);
void stack_pop();
char* stack_register_var(char *var_name, ast_node_ptr node);
void stack_resolve_id(char *buffer, char *var_name);

//List functions
void save_var_id(ast_node_ptr node, char *gen_id);
char* find_var_id(ast_node_ptr node);
void cleanup_ids();
void stack_push_existing(char *var_name, char *unique_id, ast_node_ptr node);
void scan_variables(ast_node_ptr node);

//Scope, function calling
void print_call(char* label);
void print_defvar(char* var_name);
void print_move( char* destination, char* source);

//Instructions working with stack
void print_pushs(SYM_TYPE type, char* value, char* frame);
void print_pops(char* var);
void print_clears();

//Arithmetic, relation, boolean and conversion instruction
void print_arithmetic(ARITHMETIC arithmetic);
void print_relation(RELATION relation);
void print_boolean(BOOLEAN boolean);
void print_conversion(CONVERSION conversion);

//In-Out instructions
void print_read(char* var, char* type);
void print_write(char* symbol);

//String instructions
void print_concat(char* var, char* symbol_1, char* symbol_2);
void print_strlen(char* var, char* symbol);
void print_getchar(char* var, char* symbol, char* index);
void print_setchar(char* var, char* index, char* symbol);

//Type instructions
void print_types();
void print_isint_isints();

//Program flow instructions
void print_label(char* label);
void print_jump(char* label);
void print_jumpifeqs(char* label);
void print_jumpifneqs(char* label);
void print_exit(char* symbol);

//Debugging instructions
void print_break();
void print_dprint(char* symbol);

#endif //GENERATOR_H