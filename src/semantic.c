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
#include <stdlib.h>

typedef struct {
    const char *name;
    DataType return_type;
    int param_num;
    const DataType *param_type;
} builtin_t;

static const builtin_t builtins[] = {
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

/**
 * Note: vygenerovane pomocou Claude opus s prommptom: Napis funkciu make_unique_name,
 * ktora vrati alokovany retazec v tvare meno$scope_id, NULL pri chybe alokacie.
 *
 * Vytvori unikatne meno premennej pre generator v tvare "meno$scope_id".
 * Jedinecne je preto, ze v jednom rozsahu nemozu byt dve rovnake mena (chyba 4)
 * a scope_id je jedinecne pre kazdy blok v celom programe.
 * Deklaracia aj kazde pouzitie premennej volaju funkciu s rovnakym scope_id,
 * takze vzdy vznikne rovnake meno.
 * @param name povodne meno premennej
 * @param scope_id cislo rozsahu v ktorom bola premenna deklarovana
 * @return novo alokovany retazec (uvolnuje ast_free), NULL pri chybe alokacie
 */
static char *make_unique_name(const char *name, int scope_id) {
    if (name == NULL) return NULL;

    int len = snprintf(NULL, 0, "%s$%d", name, scope_id);
    if (len < 0) return NULL;

    char *unique = malloc((size_t)len + 1);
    if (unique == NULL) return NULL;

    snprintf(unique, (size_t)len + 1, "%s$%d", name, scope_id);
    return unique;
}
static bool assign_conv_type(DataType target, ASTNode *value) {
    DataType src = value->data_type;
    if (target == TYPE_ANY) return true;
    if (src == TYPE_BOOL) return false;
    if (target == src) return true;
    if (target == TYPE_DOUBLE && src == TYPE_INT) {
        value->to_double = true;
        return true;
    }
    return false;
}
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
        data.func.is_main = function->is_main;
        data.func.defined = true;
        data.func.return_explicit = (function->data_type != TYPE_NONE);
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

    if (main_count == 0) return SEM_VAL_UNDEFINED_ERR;
    if (main_count > 1) return SEM_REDEF_ERR;

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


    int error = first_run(root, global);
    if (error == ERROR_OK) error = second_run(root, global);
    free_stack(stack);
    return error;
}