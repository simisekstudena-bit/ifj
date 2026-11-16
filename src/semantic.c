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

typedef struct {
    const char *name;
    DataType return_type;
    int param_num;
    const DataType *param_type;
} builtin_t;

const builtin_t builtins[] = {
    {"print", TYPE_UNIT, 1, (DataType[]){ TYPE_ANY }},
    {"readLine", TYPE_STRING, 0, NULL},
    {"readInt", TYPE_INT, 0, NULL},
    {"readDouble", TYPE_DOUBLE, 0, NULL},
    {"toStr", TYPE_STRING, 1, (DataType[]){ TYPE_ANY }},
    {"toInt", TYPE_INT, 1, (DataType[]){ TYPE_DOUBLE}},
    {"strLength", TYPE_INT, 1, (DataType[]){ TYPE_STRING }},
    {"subStr", TYPE_STRING, 3, (DataType[]){ TYPE_STRING, TYPE_INT, TYPE_INT }},
    {"ord", TYPE_INT, 2, (DataType[]){ TYPE_STRING, TYPE_INT }},
    {"chr", TYPE_STRING, 1, (DataType[]){ TYPE_INT }},
};

static const char *param_names[] = { "p0", "p1", "p2" };

int first_run(ASTNode *root, symtable_t *global) {
    const size_t count = sizeof(builtins) / sizeof(builtins[0]);
    for (size_t i = 0; i < count; i++) {
        symtable_data_t data = {0};
        data.kind = SYMBOL_FUNCTION;
        data.data_type = builtins[i].return_type;
        data.func.builtin = true;
        data.func.defined = true;
        data.func.return_explicit = true;
        if (insert_data(global, builtins[i].name, data) != SYMTABLE_OK) return INTERNAL_INTERPRET_ERR;
        symtable_data_t *table = search_item(global, builtins[i].name);
        for (int param = 0;  param < builtins[i].param_num; param++) {
            const symtable_status_t status = add_parameter(table, param_names[param], builtins[i].param_type[param]);
            if ( status != SYMTABLE_OK) {
                if (status == SYMTABLE_ERR_EXISTS) return SEM_REDEF_ERR;
                return INTERNAL_INTERPRET_ERR;
            }
        }
    }

    int main_count = 0;

    for (ASTNode *function = root->body; function != NULL; function = function->next) {
        symtable_data_t data = {0};
        data.kind = SYMBOL_FUNCTION;
        data.data_type = function->data_type;
        if (function->data_type != TYPE_NONE) data.data_type = function->data_type;
        data.func.is_main = function->is_main;
        data.func.defined = true;
        symtable_status_t  err;
        if ((err = insert_data(global, function->name, data)) != SYMTABLE_OK) {
            if (err == SYMTABLE_ERR_EXISTS)
                return SEM_REDEF_ERR;
            return INTERNAL_INTERPRET_ERR;
        }
        symtable_data_t *table = search_item(global, function->name);
        for (ASTNode *param = function->params; param != NULL; param = param->next) {
            symtable_status_t status = add_parameter(table, param->name, param->data_type);
            if ( status != SYMTABLE_OK) {
                if (status == SYMTABLE_ERR_EXISTS) return SEM_REDEF_ERR;
                return INTERNAL_INTERPRET_ERR;
            }
        }
        if (function->is_main) main_count++;
    }

    if (main_count != 1) return SEM_OTHER_ERROR;

    return ERROR_OK;
}

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
    int err_first_run = first_run(root, global);
    free_stack(stack);
    return err_first_run;
}