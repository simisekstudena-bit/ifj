/*
@file ast.h
@author Simona Studená (xstudes00)
*/
#ifndef AST_H
#define AST_H

#include "scanner.h"
#include "types.h"
#include <stdbool.h>

/*
@brief seznam možných typů uzlů
v komentářích jsou součásti, potomci
*/
typedef enum {
    NODE_PROGRAM,        // seznam funkcí v body
    NODE_FUNC_DEF,       // name, params, opt data_type, body, is_main
    NODE_FUNC_CALL,      // name, params jako argumenty
    NODE_PARAM,          // name, data_type
    NODE_BLOCK,          // první příkaz v body, další příkazy v ukazateli next
    NODE_VAR_DECL,       // name, is_var, data_type, inicializace v left
    NODE_ASSIGN,         // name, příkaz v left
    NODE_IF,             // cond, body then, else body
    NODE_WHILE,          // cond, body
    NODE_RETURN,         // výraz k navrácení v left
    NODE_EXPR_STMT,      // výraz v left
    NODE_INT_LITERAL,    // int_value
    NODE_DOUBLE_LITERAL, // double_value
    NODE_STRING_LITERAL, // string_value
    NODE_UNIT_LITERAL,
    NODE_IDENTIFIER, // name
    NODE_BINOP,      // op, left, right
    NODE_UNARY_OP    // op, right
} NodeType;

/*
@brief struktura uzlu ast
*/
typedef struct ASTNode {
    NodeType type;
    struct ASTNode *next; // ukazatel na další

    struct ASTNode *left;      // left operand
    struct ASTNode *right;     // right operand
    struct ASTNode *cond;      // podmínka pro uzly if a while
    struct ASTNode *body;      // body (then větev, loop, block)
    struct ASTNode *else_body; // else větev
    struct ASTNode *params;    // parametery, argumenty funkce

    char *name;
    char *string_value;
    int int_value;
    double double_value;

    TokenType op; // operator
    DataType data_type;
    bool is_var; // jestli je var (1) nebo val (0)
    bool is_main;

    char *unique_name;
    bool to_double;
    struct ASTNode *function_definition;
    bool builtin;

} ASTNode;

ASTNode *ast_create_node(NodeType type);

ASTNode *ast_append(ASTNode *head, ASTNode *node);

void ast_free(ASTNode *node);

#endif