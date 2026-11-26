/*
@file parser_internal.h
@author Simona Studená (xstudes00)
@brief shared parser state and helpers used by parser.c and expression.c
*/

#ifndef PARSER_INTERNAL_H
#define PARSER_INTERNAL_H

#include "scanner.h"
#include "ast.h"

extern Token current_token;     //current token to be parsed
void advance(void);            // reads advance token, respects lookahead
TokenType peek_type(void);      // peeks for lookaheads token type
void syntax_error(const char *message);     // writes errors to stderr aned exits with code 2
ASTNode *parse_func_call(void);

#endif