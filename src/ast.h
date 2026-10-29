#ifndef AST_H
#define AST_H

#include "scanner.h"
#include <stdbool.h>

typedef enum {
    NODE_PROGRAM,
    NODE_VAR_DECL,
    NODE_ASSIGN,
    NODE_PARAM,
    NODE_INT_LITERAL,
    NODE_DOUBLE_LITERAL,
    NODE_STRING_LITERAL,
    NODE_UNIT_LITERAL,
    NODE_IDENTIFIER,
    NODE_BINOP,
    NODE_IF,
    NODE_WHILE,
    NODE_RETURN,
    NODE_FUNC_DEF,
    NODE_FUNC_CALL
} NodeType;

struct ASTNode;

typedef struct ASTNode {
    NodeType type;
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *cond;
    struct ASTNode *body;

    char *name;
    bool is_var;            //jestli je var (1) nebo val (0)
    char *data_type;

    TokenAtribut attribute;
} ASTNode;

ASTNode* ast_create_node(NodeType type);
void ast_free(ASTNode *node);

#endif