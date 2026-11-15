/*
Projekt: Implementace prekladace
author: Jakub Ferencik xferenj00
Kod semantickej analyzy
*/

#include "semantic.h"
#include "symtable.h"
#include "ast.h"
#include "errors.h"
#include <stdio.h>

int semantic_analysis(ASTNode *root) {
    if (root == NULL || root->type != NODE_PROGRAM) return INTERNAL_INTERPRET_ERR;

    symtable_stack_t *stack = init_stack();
    if (stack == NULL) return INTERNAL_INTERPRET_ERR;

    symtable_t *global = create_table(32); // uchovava definicie
    if (global == NULL) {
        free_stack(stack);
        return INTERNAL_INTERPRET_ERR;
    }

    if (stack_push(stack, global) != SYMTABLE_OK) {
        free_stack(stack);
        free_symtable(global);
        return INTERNAL_INTERPRET_ERR;
    }
}