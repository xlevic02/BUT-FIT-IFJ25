#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

FILE *file;
static int label_counter = 0;

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
    }CONVERSTION;

//Ramec, volanie funkcii
void print_createframe(FILE *file);
void print_pushframe(FILE *file);
void print_popframe(FILE *file);
void print_call(FILE *file, char* label);
void print_return(FILE *file);
void print_defvar(FILE *file, char* var_name);
void print_move(FILE *file, char* source, char* destination);

//Instructions working with stack
void print_pushs(FILE *file, char* symbol);
void print_pops(FILE *file, char* var);
void print_clears(FILE *file);

//Arithmetic, relation, boolean and conversion instruction
void print_arithmetic(FILE *file, ARITHMETIC arithmetic);
void print_relation(FILE *file, RELATION relation);
void print_boolean(FILE *file, BOOLEAN boolean);
void print_conversion(FILE *file, CONVERSTION conversion);

//In-Out instructions
void print_read(FILE *file, char* type, char* var);
void print_write(FILE *file, char* symbol);

//String instructions
void print_concat(FILE *file, char* var, char* symbol_1, char* symbol_2);
void print_strlen(FILE *file, char* var, char* symbol);
void print_getchar(FILE *file, char* var, char* symbol, char* index);
void print_setchar(FILE *file, char* var, char* index, char* symbol);

//Type instructions
void print_types(FILE *file);
void print_isint_isints(FILE *file);

//Program flow instructions
void print_label(FILE *file, char* label);
void print_jump(FILE *file, char* label);
void print_jumpifeqs(FILE *file, char* label);
void print_jumpifneq(FILE *file, char* label);
void print_exit(FILE *file, char* symbol);

//Debugging instructions
void print_break(FILE *file);
void print_dprint(FILE *file, char* symbol);
