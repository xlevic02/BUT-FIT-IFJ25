#include "generator.h"


void print_createframe(FILE *file)
    {
        fprintf(file, "CREATEFRAME\n");
    }

void print_pushframe(FILE *file)
    {
        fprintf(file, "PUSHFRAME\n");
    }

void print_popframe(FILE *file)
    {
        fprintf(file, "POPFRAME\n");
    }

void print_call(FILE *file, char* label)
    {
        fprintf(file, "CALL %s\n", label);
    }

void print_return(FILE *file)
    {
        fprintf(file, "RETURN\n");
    }

void print_defvar(FILE *file, char* var_name)
    {
        fprintf(file, "DEFVAR %s\n", var_name);
    }

void print_move(FILE *file, char* source, char* destination)
    {
        fprintf(file, "MOVE %s %s\n", destination, source);
    }

void print_pushs(FILE *file, char* symbol)
    {
        fprintf(file, "PUSHS %s\n", symbol);
    }
void print_pops(FILE *file, char* var)
    {
        fprintf(file, "POPS %s\n", var);
    }

void print_clears(FILE *file)
    {
        fprintf(file, "CLEARS\n");
    }

void print_arithmetic(FILE *file, ARITHMETIC arithmetic)
    {
        switch(arithmetic)
            {
                case ADD:
                    fprintf(file, "ADDS\n");
                    break;
                case SUB:
                    fprintf(file, "SUBS\n");
                    break;
                case MUL:
                    fprintf(file, "MULS\n");
                    break;
                case DIV:
                    fprintf(file, "DIVS\n");
                    break;
                case IDIV:
                    fprintf(file, "IDIVS\n");
                    break;
                default:
                    fprintf(stderr, "Error, non existent operation\n");
                    return 1;
            }
                
    }

void print_relation(FILE *file, RELATION relation)
    {
        switch(relation)
            {
                case LT:
                    fprintf(file, "LTS\n");
                    break;
                case GT:
                    fprintf(file, "GTS\n");
                    break;
                case EQ:
                    fprintf(file, "EQS\n");
                    break;
                default:
                    fprintf(stderr, "Error, non existent operation\n");
                    return 1;
            }
    }

void print_boolean(FILE *file, BOOLEAN boolean)
    {
        switch(boolean)
            {
                case AND:
                    fprintf(file, "ANDS\n");
                    break;
                case OR:
                    fprintf(file, "ORS\n");
                    break;
                case NOT:
                    fprintf(file, "NOTS\n");
                    break;
                default:
                    fprintf(stderr, "Error, non existent operation\n");
                    return 1;
            }
    }

void print_conversion(FILE *file, CONVERSTION conversion)
    {
        switch(conversion)
            {
                case INT2FLOAT:
                    fprintf(file, "INT2FLOATS\n");
                    break;
                case FLOAT2INT:
                    fprintf(file, "FLOAT2INTS\n");
                    break;
                case INT2CHAR:
                    fprintf(file, "INT2CHARS\n");
                    break;
                case STRI2INT:
                    fprintf(file, "STRI2INTS\n");
                    break;
                case FLOAT2STR:
                    fprintf(file, "FLOAT2STRS\n");
                    break;
                case INT2STR:
                    fprintf(file, "INT2STRS\n");
                    break;
                default:
                    fprintf(stderr, "Error, non existent operation\n");
                    return 1;
            }
    }

void print_read(FILE *file, char* type, char* var)
    {
        fprintf(file, "READ %s %s\n", var, type);
    }

void print_write(FILE *file, char* symbol)
    {
        fprintf(file, "WRITE %s\n", symbol);
    }

void print_concat(FILE *file, char* var, char* symbol_1, char* symbol_2)
    {
        fprintf(file, "CONCAT %s %s %s\n", var, symbol_1, symbol_2);
    }

void print_strlen(FILE *file, char* var, char* symbol)
    {
        fprintf(file, "STRLEN %s %s\n", var, symbol);
    }

void print_getchar(FILE *file, char* var, char* symbol, char* index)
    {
        fprintf(file, "GETCHAR %s %s %s\n", var, symbol, index);
    }

void print_setchar(FILE *file, char* var, char* index, char* symbol)
    {
        fprintf(file, "SETCHAR %s %s %s\n", var, index, symbol);
    }

void print_types(FILE *file)
    {
        fprintf(file, "TYPES\n");
    }

void print_isint_isints(FILE *file)
    {
        fprintf(file, "ISINTS\n");
    }

void print_label(FILE *file, char* label)
    {
        fprintf(file, "LABEL %s\n", label);
    }

void print_jump(FILE *file, char* label)
    {
        fprintf(file, "JUMP %s\n", label);
    }

void print_jumpifeqs(FILE *file, char* label, char* symbol_1, char* symbol_2)
    {
        fprintf(file, "JUMPIFEQS %s %s %s\n", label, symbol_1, symbol_2);
    }

void print_jumpifneqs(FILE *file, char* label)
    {
        fprintf(file, "JUMPIFNEQS %s %s %s\n", label);
    }

void print_exit(FILE *file, char* symbol)
    {
        fprintf(file, "EXIT %s\n", symbol);
    }

void print_break(FILE *file)
    {
        fprintf(file, "BREAK\n");
    }
void print_dprint(FILE *file, char* symbol)
    {
        fprintf(file, "DPRINT %s\n", symbol);
    }