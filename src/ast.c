/*
@file: ast.c
@author: Simona Studená (xstudes00)
*/
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

/*
@brief vytvoří nový AST uzel konkrétního typu
@param type typ uzlu
@return node ukazatel na nový uzel
*/
ASTNode *ast_create_node(NodeType type) {
    ASTNode *node = (ASTNode *)calloc(1, sizeof(ASTNode));
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(99);
    }
    node->type = type;
    node->data_type = TYPE_NONE;
    return node;
}

/*
@brief připojí ast uzel
@param head ukazatel na začátek stromu
@param node ukazatel na uzel k připojení
@return ukazatel na začátek ast
*/
ASTNode *ast_append(ASTNode *head, ASTNode *node) {
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
@brief uvolňuje paměť alokovanou pro uzel a jeho potomky
@param node ukazatel na uzel, který má být uvolněn
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

        if (node->name != NULL) {
            free(node->name);
        }

        if (node->string_value != NULL) {
            free(node->string_value);
        }

        free(node->unique_name);

        free(node);

        node = next;
    }
}
