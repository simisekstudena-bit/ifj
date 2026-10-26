/*
Projekt: Implementace prekladace
Author: Patrik Kadleček xkadlep01

*/
#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdlib.h>
#include <stdio.h>

// Operators supported in expressions
typedef enum {
    OP_ADD,     // x+y
    OP_SUB,     // x-y
    OP_DIV,     // x/y
    OP_MUL,     // x*y
    OP_NEG,     // -x
    OP_GT,      // x>y
    OP_GTE,     // x>=y
    OP_LT,      // x<y
    OP_LTE,     // x<=y
    OP_EQ,      // x==y
    OP_NEQ      // x!=y
} operator_t;

// Initializes generator
void gen_init(void);

// Writes generated program to stdout
void gen_print(FILE *out);

// Frees memory used by generator
void gen_free(void);

#endif
