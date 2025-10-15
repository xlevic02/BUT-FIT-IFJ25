// Implementace prekladace imperativniho jazyka IFJ25
// Error by Jan Frantisek "xlevic02" Levicek on 10/3/24.
//

#include<stdio.h>
#include<stdlib.h>
#include "error.h"

void error(int err_num, const char* err_message) {
    fprintf(stderr,"%s", err_message);
    exit(err_num);
}