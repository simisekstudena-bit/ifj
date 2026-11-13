/*
Projekt: Implementace prekladace
Author: Patrik Kadleček xkadlep01

*/
#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdlib.h>
#include <stdio.h>

// Initializes generator
void gen_init(void);

// Writes generated program to stdout
void gen_print(FILE *out);

// Frees memory used by generator
void gen_free(void);

#endif
