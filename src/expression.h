/*
@author Simona Studená (xstudes00)
@file expression.h
*/
#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <stdbool.h>
#include "ast.h"
#include "scanner.h"

/*
terminals according to the precedence table
*/
typedef enum {
    SYM_UNARY_MINUS,    // u-
    SYM_ADD_SUB,        // +,-
    SYM_MUL_DIV,        // *,/
    SYM_REL,             // < <= > >=
    SYM_EQ,             // ==, !=
    SYM_LPAREN,         // (
    SYM_RPAREN,         // )
    SYM_TERM,           // literl or id
    SYM_END             // $
} PrecSymbol;

/*
@brief parses expression from current_token
    after parsing current_token is the first token out of the expression
*/
ASTNode *parse_expression(void);

#endif