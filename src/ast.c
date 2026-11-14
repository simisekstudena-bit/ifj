/*
@file: ast.c
@author: Simona Studená (xstudes00)
*/
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

/*
@brief creates a new AST node with the specified type
@param type the type of the AST node
@return pointer to the newly created AST node
*/
ASTNode* ast_create_node(NodeType type) {
    ASTNode *node = (ASTNode*) malloc(sizeof(ASTNode));
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(99);
    }
    node->type = type;
    node->data_type = TYPE_NONE;
    return node;
}

/*
@brief appends a node to the end of the abstract syntax tree
@param head pointer to the head of the AST
@param node pointer to the node to be appended
@return pointer to the head of the AST
*/
ASTNode *ast_append(ASTNode *head, ASTNode *node){
    if (head == NULL) {
        return node;
    }
    ASTNode *current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = node;
    return head;
}

/*
@brief frees the memory allocated for an AST node and its children
@param node pointer to the AST node to be freed
*/
void ast_free(ASTNode *node) {
    while (node != NULL) {
        ASTNode *next = node->next;
        ast_free(node->left);
        ast_free(node->right);
        ast_free(node->cond);
        ast_free(node->body);
        ast_free(node->else_body);
        ast_free(node->params);

        if(node->name != NULL) {
            free(node->name);
        }

        if(node->string_value != NULL) {
            free(node->string_value);
        }

        free(node);

        node = next;
    }
}
