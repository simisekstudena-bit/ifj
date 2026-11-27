/*
Projekt: Implementace prekladace
author: Jakub Ferencik xferenj00
Rozhranie semantickej analyzy
*/

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "errors.h"
#include "symtable.h"

/**
 * vstupny bod semantiky ktory sa vola z mainu.
 * Doplni do AST data_type, to_double, unique_name, func_def a is_buitin
 * @param root root uzol z parsera
 * @return 0 alebo chybovy kod TODO: vytvorit errors.h
 */
int semantic_analysis(ASTNode *root);

/**
 * Todo: Marek
 * skontroluje vyraz a kazdemu uzlu podstromu vyplni data_type, pri miesani Int a double
 * nastavi to_double na int op
 * type bool iba vrati a ci je na dannom mieste povoleny rozhodne volajuci
 * @param expression root node vyrazu
 * @param scopes aktualny zasobnik rozsahov
 * @return 0 alebo chybovy kod
 */
int sem_expression(ASTNode *expression, symtable_stack_t *scopes);

/**
 * Najde premennu podla mena a vyplni unikatne meno
 * @param scopes zasobnik rozsahov
 * @param node uzol s vyplnenym menom
 * @param found if not null, dostane data najdeneho symbolu
 * @return 0 alebo chybovy kod
 */
int sem_lookup_var(symtable_stack_t *scopes, ASTNode *node, symtable_data_t **found);

/**
 * Kontroluje volanie funkcie -- existenciu, pocet a typy argumentov
 * Argumentom vyplni data_type a to double, volaniu data_type, func_def a is_builtin
 * @param node to check
 * @param scopes zasobnik rozsahov
 * @return 0 alebo chybovy kod z err
 */
int sem_call(ASTNode *node, symtable_stack_t *scopes);

#endif