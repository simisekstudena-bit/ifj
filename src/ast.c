#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

ASTNode* ast_create_node(NodeType type) {
    ASTNode *node = (ASTNode*) malloc(sizeof(ASTNode));
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(99);
    }
    node->type = type;

    node->left = NULL;
    node->right = NULL;
    node->cond = NULL;
    node->body = NULL;
    node->name = NULL;

    return node;
}
void ast_free(ASTNode *node) {
    if (node == NULL) return;
    ast_free(node->left);
    ast_free(node->right);
    ast_free(node->cond);
    ast_free(node->body);

    if(node->name != NULL) {
        free(node->name);
    }

    free(node);
}
