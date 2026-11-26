/*
@file parser_internal.h
@author Simona Studená (xstudes00)
@brief sdílené prostředky pro parser.c a expression.c
*/

#ifndef PARSER_INTERNAL_H
#define PARSER_INTERNAL_H

#include "scanner.h"
#include "ast.h"

extern Token current_token;     // aktuální token
void advance(void);            // čte další token s ohledem na lookahead pokud je
TokenType peek_type(void);      // vrací typ dalšího tokenu pokud lookahead
void syntax_error(const char *message);     // vypisuje na stderr v případě chyby
ASTNode *parse_func_call(void);     // zpracovává volání funkce pro rozšíření funexp

#endif