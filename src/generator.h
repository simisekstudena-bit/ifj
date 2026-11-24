/*
@file generator.h
@author Patrik Kadleček (xkadlep01)
@brief Generator of IFJcode26 from the annotated AST
*/

#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>
#include "ast.h"

/*
@brief initializes the instruction list and writes the .IFJcode26 header
*/
void gen_init(void);

/*
@brief writes the generated program, one instruction per line
@param out output file (stdout in the compiler)
*/
void gen_print(FILE *out);

/*
@brief frees all generated instructions
*/
void gen_free(void);

/*
@brief generates code for the whole program (built-ins and all functions)
@param root NODE_PROGRAM after semantic analysis
*/
void gen_program(ASTNode *root);
#endif
