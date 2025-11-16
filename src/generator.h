// Implementace generatoru vysledneho kodu IFJcode25
//generator.h by Marek "xbalism00" Bališ on 11/11/25

#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static int label_counter = 0;
static char label_stack[100][32];
static int label_stack_top = -1;
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

//Function to generate code for an expression
void generate_expression(ast_node_ptr node, bst_scope_ptr *current_scope);

//Main function for generating code
int generate_code(ast_node_ptr root, bst_scope_ptr symtable);

//Function for getting the name of a variable
char* get_variable_id(bst_scope_ptr scope, char* lexeme);

//Funkction to get a frame for a variable from symtable
char* get_variable_frame(bst_scope_ptr scope, char* var_name);

//Creates a unique label for if, else, while
void get_unique_label(char* buffer, const char* prefix);

//Function to format string for IFJcode25
char* format_string_for_ifjcode(const char* lexeme);

//Function to format float for IFJcode25
char* format_float_for_ifjcode(const char* lexeme);

//Placeholder functions since our semantic analysis is not done yet
token_type_t generator_get_id_type(char* lexeme, bst_scope_ptr* current_scope);
token_type_t generator_get_type(ast_node_ptr node, bst_scope_ptr* current_scope);

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
void print_read(char* type, char* var);
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