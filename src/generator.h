/*
Projekt: Implementace prekladace
Author: Patrik Kadleček xkadlep01

*/
#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>
#include "ast.h"

// Initializes generator
void gen_init(void);

// Writes generated program to stdout
void gen_print(FILE *out);

// Frees memory used by generator
void gen_free(void);

// Generates final code from AST root
void gen_program(ASTNode *root);

#endif
