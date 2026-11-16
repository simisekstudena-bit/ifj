/*
@file ast.h
@author Simona Studená (xstudes00)
*/
#ifndef AST_H
#define AST_H

#include "scanner.h"
#include <stdbool.h>
#include "types.h"

/*
@brief enumeration of possible node types
components of each node are in the comments
*/
typedef enum {
    NODE_PROGRAM,       // list of functions in body
    NODE_FUNC_DEF,      //name, params, opt data_type, body, is_main
    NODE_FUNC_CALL,     //name, params used as arguments
    NODE_PARAM,         //name, data_type
    NODE_BLOCK,         // first statement in body, following statements through pointer next
    NODE_VAR_DECL,      // name, is_var, data_type, inicialization in left pointer
    NODE_ASSIGN,        //name, statement in left pointer
    NODE_IF,            // cond, body then, else body
    NODE_WHILE,         // cond, body
    NODE_RETURN,        // expression to return in left pointer
    NODE_EXPR_STMT,     // expression in left pointer
    NODE_INT_LITERAL,   // int_value    
    NODE_DOUBLE_LITERAL, //double_value    
    NODE_STRING_LITERAL, // string_value   
    NODE_UNIT_LITERAL,   
    NODE_IDENTIFIER,    // name
    NODE_BINOP,         // op, left, right
    NODE_UNARY_OP       // op, right
} NodeType;

/*
@brief structure representing a node
*/
typedef struct ASTNode {
    NodeType type;
    struct ASTNode *next;       // pointer to the next node

    struct ASTNode *left;       // left operand
    struct ASTNode *right;      // right operand
    struct ASTNode *cond;       // condition for if and while nodes
    struct ASTNode *body;       // body (then branch, loop, block)
    struct ASTNode *else_body;  // else branch
    struct ASTNode *params;     // function parameters, arguments

    char *name;                 
    char *string_value;
    int int_value;
    double double_value;
    
    TokenType op;           // operator
    DataType data_type;
    bool is_var;            //jestli je var (1) nebo val (0)
    bool is_main;

    char *unique_name;
    bool to_double;
    struct ASTNode *function_definition;
    bool builtin;

} ASTNode;

ASTNode* ast_create_node(NodeType type);

ASTNode *ast_append(ASTNode *head, ASTNode *node);

void ast_free(ASTNode *node);

#endif