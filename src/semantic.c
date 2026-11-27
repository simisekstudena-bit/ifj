/*
Projekt: Implementace prekladace
author: Jakub Ferencik xferenj00
Kod semantickej analyzy
*/

#include "semantic.h"
#include "ast.h"
#include "errors.h"
#include "symtable.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    const char *name;
    DataType return_type;
    int param_num;
    const DataType *param_type;
} builtin_t;

static const builtin_t builtins[] = {
    {"print", TYPE_UNIT, 1, (DataType[]){TYPE_ANY}},
    {"readLine", TYPE_STRING, 0, NULL},
    {"readInt", TYPE_INT, 0, NULL},
    {"readDouble", TYPE_DOUBLE, 0, NULL},
    {"toStr", TYPE_STRING, 1, (DataType[]){TYPE_ANY}},
    {"toInt", TYPE_INT, 1, (DataType[]){TYPE_DOUBLE}},
    {"strLength", TYPE_INT, 1, (DataType[]){TYPE_STRING}},
    {"subStr", TYPE_STRING, 3, (DataType[]){TYPE_STRING, TYPE_INT, TYPE_INT}},
    {"ord", TYPE_INT, 2, (DataType[]){TYPE_STRING, TYPE_INT}},
    {"chr", TYPE_STRING, 1, (DataType[]){TYPE_INT}},
};

static const char *param_names[] = {"p0", "p1", "p2"};

static symtable_t *g_global = NULL;
static int check_function(ASTNode *function);
static int sem_check_block(ASTNode *block, symtable_stack_t *stack, symtable_data_t *function_data);
static int check_return(ASTNode *value, symtable_stack_t *stack, symtable_data_t *function_data);

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

static int sem_expression_extraction(ASTNode *expression, symtable_stack_t *stack) {
    if (expression->type == NODE_FUNC_CALL) return sem_call(expression, stack);
    return sem_expression(expression, stack);
}

static bool assign_conv_type(DataType target, ASTNode *value) {
    DataType src = value->data_type;
    if (src == TYPE_BOOL) return false;
    if (target == TYPE_ANY) return true;
    if (target == src) return true;
    if (target == TYPE_DOUBLE && src == TYPE_INT) {
        value->to_double = true;
        return true;
    }
    return false;
}

int sem_call(ASTNode *node, symtable_stack_t *scopes) {
    symtable_data_t *data = search_item(g_global, node->name);
    if (data == NULL) return SEM_VAL_UNDEFINED_ERR;
    int num_arguments = 0;
    for (ASTNode *param = node->params; param != NULL; param = param->next)
        num_arguments++;
    if (num_arguments != data->func.param_count) return SEM_CALL_ERR;
    int argument_index = 0;
    for (ASTNode *param = node->params; param != NULL; param = param->next) {
        int err = sem_expression(param, scopes);
        if (err != ERROR_OK) return err;
        if (!assign_conv_type(data->func.params[argument_index].type, param)) return SEM_CALL_ERR;
        argument_index++;
    }
    if (data->data_type == TYPE_NONE) {
        if (data->func.inferring) return SEM_DIFF_TYPE_ERR;
        int err = check_function(data->func.node);
        if (err != ERROR_OK) return err;
    }
    node->data_type = data->data_type;
    node->function_definition = data->func.node;
    node->builtin = data->func.builtin;

    return ERROR_OK;
}

/**
 * vygenerovane pomocou claude opus s promptom: Vytvor funkciu declare_val ktora zapise data val do
 * symtable a prida nodu unique name
 */
static int declare_val(symtable_stack_t *stack, ASTNode *node, symtable_kind_t kind,
                       bool modifiable) {
    symtable_data_t data = {0};
    data.kind = kind;
    data.data_type = node->data_type;
    data.var.is_modifiable = modifiable;
    data.var.scope_id = stack_scope_id(stack);
    symtable_status_t status = insert_data(stack_top(stack), node->name, data);
    if (status == SYMTABLE_ERR_EXISTS) return SEM_REDEF_ERR;
    if (status != SYMTABLE_OK) return INTERNAL_INTERPRET_ERR;
    node->unique_name = make_unique_name(node->name, data.var.scope_id);
    if (node->unique_name == NULL) return INTERNAL_INTERPRET_ERR;
    return ERROR_OK;
}

static ASTNode *last_statement(ASTNode *block) {
    ASTNode *last = NULL;
    for (ASTNode *statement = block->body; statement != NULL; statement = statement->next) {
        last = statement;
    }
    return last;
}

static bool ends_with_return(ASTNode *block) {
    ASTNode *last = last_statement(block);
    if (last == NULL) return false;
    if (last->type == NODE_RETURN) return true;
    if (last->type == NODE_IF)
        return ends_with_return(last->body) && ends_with_return(last->else_body);
    return false;
}

static bool result_is_unit(ASTNode *block) {
    ASTNode *last = last_statement(block);
    if (last == NULL) return true;
    switch (last->type) {
    case NODE_EXPR_STMT:
        return last->left->data_type == TYPE_UNIT;
    case NODE_IF:
        return result_is_unit(last->body) && result_is_unit(last->else_body);
    case NODE_RETURN:
        return false;
    default:
        return true;
    }
}

static int check_body_result(ASTNode *body, symtable_data_t *func) {
    bool explicit_type = func->func.return_explicit;
    if (explicit_type && func->data_type == TYPE_UNIT) return ERROR_OK;
    ASTNode *last = last_statement(body);
    ASTNode *value = NULL;
    if (last != NULL) {
        if (last->type == NODE_RETURN) return ERROR_OK;
        if (last->type == NODE_EXPR_STMT) value = last->left;
        if (last->type == NODE_IF) {
            if (explicit_type) {
                if (ends_with_return(body)) return ERROR_OK;
                return SEM_DIFF_TYPE_ERR;
            }
            if (!result_is_unit(body)) return SEM_DIFF_TYPE_ERR;
            func->data_type = TYPE_UNIT;
            return ERROR_OK;
        }
    }

    if (!explicit_type) {
        if (value != NULL)
            func->data_type = value->data_type;
        else
            func->data_type = TYPE_UNIT;
        return ERROR_OK;
    }

    if (value == NULL || !assign_conv_type(func->data_type, value)) return SEM_DIFF_TYPE_ERR;
    return ERROR_OK;
}

static ASTNode *make_bock(ASTNode *expression) {
    ASTNode *statement = ast_create_node(NODE_EXPR_STMT);
    statement->left = expression;
    ASTNode *block = ast_create_node(NODE_BLOCK);
    block->body = statement;
    return block;
}

static int check_function(ASTNode *function) {
    symtable_data_t *data = search_item(g_global, function->name);
    if (data == NULL) return INTERNAL_INTERPRET_ERR;

    if (function->body->type != NODE_BLOCK) function->body = make_bock(function->body);

    data->func.inferring = true;
    symtable_stack_t *local_stack;
    if ((local_stack = init_stack()) == NULL) return INTERNAL_INTERPRET_ERR;
    symtable_t *func_table = create_table(16);
    if (func_table == NULL || stack_push(local_stack, func_table) != SYMTABLE_OK) {
        free_symtable(func_table);
        free_stack(local_stack);
        return INTERNAL_INTERPRET_ERR;
    }
    int err = ERROR_OK;
    for (ASTNode *param = function->params; param != NULL && err == ERROR_OK; param = param->next) {
        err = declare_val(local_stack, param, SYMBOL_PARAMETER, false);
    }
    if (err == ERROR_OK) err = sem_check_block(function->body, local_stack, data);
    if (err == ERROR_OK) err = check_body_result(function->body, data);
    if (err == ERROR_OK && data->func.is_main && data->data_type != TYPE_UNIT)
        err = SEM_DIFF_TYPE_ERR;
    function->data_type = data->data_type;
    data->func.inferring = false;
    data->func.checked = true;
    free_stack(local_stack);

    return err;
}

int sem_lookup_var(symtable_stack_t *scopes, ASTNode *node, symtable_data_t **found) {
    symtable_data_t *data = stack_search(scopes, node->name);
    if (data == NULL) return SEM_VAL_UNDEFINED_ERR;

    node->data_type = data->data_type;
    node->unique_name = make_unique_name(node->name, data->var.scope_id);

    if (node->unique_name == NULL) return INTERNAL_INTERPRET_ERR;
    if (found != NULL) *found = data;
    return ERROR_OK;
}

static int first_run(ASTNode *root, symtable_t *global) {
    const size_t count = sizeof(builtins) / sizeof(builtins[0]);
    for (size_t i = 0; i < count; i++) {
        symtable_data_t data = {0};
        data.kind = SYMBOL_FUNCTION;
        data.data_type = builtins[i].return_type;
        data.func.builtin = true;
        data.func.return_explicit = true;
        if (insert_data(global, builtins[i].name, data) != SYMTABLE_OK)
            return INTERNAL_INTERPRET_ERR;
        symtable_data_t *table = search_item(global, builtins[i].name);
        for (int param = 0; param < builtins[i].param_num; param++) {
            const symtable_status_t status =
                add_parameter(table, param_names[param], builtins[i].param_type[param]);
            if (status != SYMTABLE_OK) {
                if (status == SYMTABLE_ERR_EXISTS) return SEM_REDEF_ERR;
                return INTERNAL_INTERPRET_ERR;
            }
        }
    }

    int main_count = 0;

    for (ASTNode *function = root->body; function != NULL; function = function->next) {
        symtable_data_t data = {0};
        data.kind = SYMBOL_FUNCTION;
        data.func.node = function;
        data.data_type = function->data_type;
        data.func.is_main = function->is_main;
        data.func.return_explicit = (function->data_type != TYPE_NONE);
        symtable_status_t err;
        if ((err = insert_data(global, function->name, data)) != SYMTABLE_OK) {
            if (err == SYMTABLE_ERR_EXISTS) return SEM_REDEF_ERR;
            return INTERNAL_INTERPRET_ERR;
        }
        symtable_data_t *table = search_item(global, function->name);
        for (ASTNode *param = function->params; param != NULL; param = param->next) {
            symtable_status_t status = add_parameter(table, param->name, param->data_type);
            if (status != SYMTABLE_OK) {
                if (status == SYMTABLE_ERR_EXISTS) return SEM_REDEF_ERR;
                return INTERNAL_INTERPRET_ERR;
            }
        }
        if (function->is_main) {
            if (function->params != NULL) return SEM_CALL_ERR;
            main_count++;
        }
    }

    if (main_count == 0) return SEM_VAL_UNDEFINED_ERR;
    if (main_count > 1) return SEM_REDEF_ERR;

    return ERROR_OK;
}

/**
 * vygenerovane pomocou claude opus s promptom: Vytvor funkciu check_statement ktora podla typu
 * prikazu skontroluje semantiku, pri var_decl najprv skontroluje hodnotu a potom zapise cez
 * declare_val, pri assign skontroluje ci premenna existuje a je var, return posle do check_return,
 * pri if a while musi byt podmienka bool a bloky skontroluje cez sem_check_block. pouzi
 * sem_expression_extraction, sem_lookup_var a assign_conv_type, chyby vracaj z errors.h
 */
static int check_statement(ASTNode *s, symtable_stack_t *stack, symtable_data_t *func) {
    int err;
    switch (s->type) {
    case NODE_VAR_DECL:
        if ((err = sem_expression_extraction(s->left, stack)) != ERROR_OK) return err;
        if (s->left->data_type == TYPE_BOOL) return SEM_DIFF_TYPE_ERR;
        if (s->data_type == TYPE_NONE)
            s->data_type = s->left->data_type;
        else if (!assign_conv_type(s->data_type, s->left))
            return SEM_DIFF_TYPE_ERR;
        return declare_val(stack, s, SYMBOL_VARIABLE, s->is_var);
    case NODE_ASSIGN: {
        symtable_data_t *var = NULL;
        if ((err = sem_lookup_var(stack, s, &var)) != ERROR_OK) return err;
        if (var->kind == SYMBOL_PARAMETER || !var->var.is_modifiable) return SEM_INITIALIZATION_ERR;
        if ((err = sem_expression_extraction(s->left, stack)) != ERROR_OK) return err;
        if (!assign_conv_type(s->data_type, s->left)) return SEM_DIFF_TYPE_ERR;
        return ERROR_OK;
    }
    case NODE_EXPR_STMT:
        if ((err = sem_expression_extraction(s->left, stack)) != ERROR_OK) return err;
        if (s->left->data_type == TYPE_BOOL) return SEM_DIFF_TYPE_ERR;
        return ERROR_OK;
    case NODE_RETURN:
        return check_return(s->left, stack, func);
    case NODE_IF:
        if ((err = sem_expression(s->cond, stack)) != ERROR_OK) return err;
        if (s->cond->data_type != TYPE_BOOL) return SEM_DIFF_TYPE_ERR;
        if ((err = sem_check_block(s->body, stack, func)) != ERROR_OK) return err;
        return sem_check_block(s->else_body, stack, func);
    case NODE_WHILE:
        if ((err = sem_expression(s->cond, stack)) != ERROR_OK) return err;
        if (s->cond->data_type != TYPE_BOOL) return SEM_DIFF_TYPE_ERR;
        return sem_check_block(s->body, stack, func);
    default:
        return INTERNAL_INTERPRET_ERR;
    }
}

static int check_return(ASTNode *value, symtable_stack_t *stack, symtable_data_t *func) {
    if (!func->func.return_explicit) return SEM_OTHER_ERROR;

    if (value == NULL) {
        if (func->data_type != TYPE_UNIT) return SEM_DIFF_TYPE_ERR;
        return ERROR_OK;
    }

    int error = sem_expression_extraction(value, stack);
    if (error != ERROR_OK) return error;
    if (value->data_type == TYPE_BOOL) return SEM_DIFF_TYPE_ERR;

    if (func->data_type == TYPE_UNIT) return ERROR_OK;
    if (!assign_conv_type(func->data_type, value)) return SEM_DIFF_TYPE_ERR;
    return ERROR_OK;
}

static int sem_check_block(ASTNode *block, symtable_stack_t *stack,
                           symtable_data_t *function_data) {
    if (block == NULL || block->type != NODE_BLOCK) return INTERNAL_INTERPRET_ERR;

    symtable_t *scope = create_table(16);
    if (scope == NULL) return INTERNAL_INTERPRET_ERR;
    if (stack_push(stack, scope) != SYMTABLE_OK) {
        free_symtable(scope);
        return INTERNAL_INTERPRET_ERR;
    }

    for (ASTNode *statement = block->body; statement != NULL; statement = statement->next) {
        int error = check_statement(statement, stack, function_data);
        if (error != ERROR_OK) return error;
    }

    stack_pop(stack);
    return ERROR_OK;
}

static int second_run(ASTNode *root, symtable_t *global) {
    g_global = global;
    for (ASTNode *function = root->body; function != NULL; function = function->next) {
        symtable_data_t *data = search_item(global, function->name);
        if (data->func.checked) continue;
        int err = check_function(function);
        if (err != ERROR_OK) return err;
    }
    return ERROR_OK;
}

int semantic_analysis(ASTNode *root) {
    if (root == NULL || root->type != NODE_PROGRAM) return INTERNAL_INTERPRET_ERR;

    symtable_t *global = create_table(32);
    if (global == NULL) return INTERNAL_INTERPRET_ERR;

    int error = first_run(root, global);
    if (error == ERROR_OK) error = second_run(root, global);

    free_symtable(global);
    return error;
}