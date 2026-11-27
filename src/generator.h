/*
Projekt: Implementace prekladace
@file generator.h
@author Patrik Kadleček (xkadlep01)
@brief rozhranie generatora kodu IFJcode26 z anotovaneho AST
*/

#ifndef GENERATOR_H
#define GENERATOR_H

#include "ast.h"
#include <stdio.h>

/*
@brief inicializuje zoznam instrukcii a zapise hlavicku .IFJcode26
*/
void gen_init(void);

/*
@brief vypise vygenerovany program, jednu instrukciu na riadok
@param out vystupny subor (v prekladaci stdout)
*/
void gen_print(FILE *out);

/*
@brief uvolni vsetky vygenerovane instrukcie
*/
void gen_free(void);

/*
@brief vygeneruje kod celeho programu (vstavane funkcie a vsetky funkcie)
@param root uzol NODE_PROGRAM po semantickej analyze
*/
void gen_program(ASTNode *root);
#endif
