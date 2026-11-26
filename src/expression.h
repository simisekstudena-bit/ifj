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
terminály odpovídající precedenční tabulce
*/
typedef enum {
    SYM_UNARY_MINUS,    // u-
    SYM_ADD_SUB,        // +,-
    SYM_MUL_DIV,        // *,/
    SYM_REL,             // < <= > >=
    SYM_EQ,             // ==, !=
    SYM_LPAREN,         // (
    SYM_RPAREN,         // )
    SYM_TERM,           // literal or id
    SYM_END             // $
} PrecSymbol;

/*
@brief
    parsuje výraz od aktuálního tokenu
    po parsování je current_token první symbol za výrazem
*/
ASTNode *parse_expression(void);

#endif