/*
Projekt: Implementace prekladace
author: Jakub Ferencik xferej00

File obsahuje source code tabulky symbolov
*/

#include "symtable.h"
#include "types.h"
#include <stdlib.h>
#include <string.h>

// horna hranica kapacity aby nepretiekol int
#define SYMTABLE_MAX_CAPACITY (1000000)
static int block_counter = 0;

typedef struct {
    symtable_data_t data;
    char *key;
} item_t;

typedef struct block {
    symtable_t *symtable;
    struct block *parent;
    int id; // jedinecne cislo rozsahu
} block_t;

struct symtable_stack {
    block_t *top;
};

struct symtable {
    item_t **items;
    int capacity;
    int count;
};

//////////////////////////////////
// funkcie na pracu nad stackom
//////////////////////////////////

bool stack_is_empty(symtable_stack_t *stack) {
    return stack == NULL || stack->top == NULL;
}

symtable_stack_t *init_stack(void) {
    symtable_stack_t *stack = malloc(sizeof(symtable_stack_t));
    if (stack == NULL) return NULL;
    stack->top = NULL;
    return stack;
}

symtable_t *stack_top(symtable_stack_t *stack) {
    if (stack_is_empty(stack) || stack->top == NULL) return NULL;
    return stack->top->symtable;
}

symtable_status_t stack_push(symtable_stack_t *stack, symtable_t *symtable) {
    if (stack == NULL || symtable == NULL) return SYMTABLE_ERR_ARGUMENT;
    block_t *block = malloc(sizeof(block_t));
    if (block == NULL) return SYMTABLE_ERR_ALLOC;

    block->symtable = symtable;
    block->parent = stack->top;
    stack->top = block;
    block_counter++;
    block->id = block_counter;
    return SYMTABLE_OK;
}

void stack_pop(symtable_stack_t *stack) {
    if (stack_is_empty(stack)) return;
    block_t *top_block = stack->top;
    stack->top = top_block->parent;

    free_symtable(top_block->symtable);
    free(top_block);
}

symtable_data_t *stack_search(symtable_stack_t *stack, const char *key) {
    if (stack_is_empty(stack)) return NULL;

    for (block_t *current_block = stack->top; current_block != NULL;
         current_block = current_block->parent) {
        symtable_data_t *data = search_item(current_block->symtable, key);
        if (data != NULL) return data;
    }
    return NULL;
}

int stack_scope_id(symtable_stack_t *stack) {
    if (stack == NULL || stack->top == NULL) return -1;
    return stack->top->id;
}

void free_stack(symtable_stack_t *stack) {
    if (stack == NULL) return;
    while (!stack_is_empty(stack)) {
        stack_pop(stack);
    }
    free(stack);
}

//////////////////////////////
// funkcie na update hodnot
/////////////////////////////

symtable_status_t add_parameter(symtable_data_t *data, const char *name, DataType type) {
    if (data == NULL || name == NULL) return SYMTABLE_ERR_ARGUMENT;
    if (data->kind != SYMBOL_FUNCTION) return SYMTABLE_ERR_ARGUMENT;
    symtable_func_t *func = &data->func;

    for (int i = 0; i < func->param_count; i++) {
        if (strcmp(func->params[i].name, name) == 0) return SYMTABLE_ERR_EXISTS;
    }

    // ked je pole plne zdvojnasobim ho
    // pri chybe alokacie tak stare pole ostane platne
    if (func->param_count == func->param_capacity) {
        int new_capacity;
        if (func->param_capacity == 0) {
            new_capacity = 4;
        } else {
            new_capacity = func->param_capacity * 2;
        }
        symtable_param_t *new_params =
            realloc(func->params, new_capacity * sizeof(symtable_param_t));
        if (new_params == NULL) return SYMTABLE_ERR_ALLOC;
        func->params = new_params;
        func->param_capacity = new_capacity;
    }

    size_t len = strlen(name) + 1;
    char *name_copy = malloc(len);
    if (name_copy == NULL) return SYMTABLE_ERR_ALLOC;
    memcpy(name_copy, name, len);

    func->params[func->param_count].name = name_copy;
    func->params[func->param_count].type = type;
    func->param_count++;

    return SYMTABLE_OK;
}

////////////////////////////////////////////////////
// pomocne funkcie na kalkulaciu velkosti tabulky
///////////////////////////////////////////////////

/**
 * Mapovacia funkcia BKDR, vypocita hash z retazca kluca
 * Algoritmus som zobral z prednaskovych slajdov IAL pre minuly rok
 *
 * @param str kluc ukonceny '\0'
 * @return hash kluca, index do tabulky je hash % capacity
 */
static unsigned int hash_bkdr(const char *str) {
    unsigned int seed = 131;
    unsigned int hash = 0;
    while (*str != '\0') {
        hash = (hash * seed) + (unsigned char)(*str);
        str++;
    }

    return hash;
}

/**
 * Zisti ci je cislo prvocislo pre volbu kapacity tabulky
 *
 * @param n testovane cislo
 * @return true ak je n prvocislo, inak false
 */
static bool is_prime(int n) {
    if (n < 2) return false;
    if (n % 2 == 0) return n == 2;

    // neparne delitele po odmocninu z n
    for (int i = 3; i <= n / i; i += 2) {
        if (n % i == 0) return false;
    }

    return true;
}

/**
 * Najde najmensie prvocislo vacsie alebo rovne n
 *
 * @param n cislo od ktoreho hladam
 * @return najblizsie prvocislo
 */
static int next_prime(int n) {
    if (n < 2) return 2;

    while (!is_prime(n)) {
        n++;
    }

    return n;
}

///////////////////////////////////////////
// Funkcie s operaciami nad tabulkou
//////////////////////////////////////////

symtable_t *create_table(int size) {
    if (size <= 0 || size > SYMTABLE_MAX_CAPACITY) return NULL;
    symtable_t *symtable = malloc(sizeof(*symtable));
    if (symtable == NULL) return NULL;

    symtable->capacity = next_prime(size);
    symtable->items = calloc(symtable->capacity, sizeof(item_t *));
    if (symtable->items == NULL) {
        free(symtable);
        return NULL;
    }

    symtable->count = 0;

    return symtable;
}

/**
 * Zvacsime tabulku na najblizsie vacsie prvocislo a potom presunieme prvky tabulky na ich nove
 * prepocitane indexy. Presuvaju sa iba pointre, takze pointre na item_t ostavaju platne.
 *
 * @param symtable tabulka ktoru zvacsujem
 * @return SYMTABLE_OK alebo chybovy stav, nove polozky aj s pridanym miesotm zostavaju
 *         pod rovnakou tabulkou
 */
static symtable_status_t resize_symtable(symtable_t *symtable) {
    if (symtable->capacity > SYMTABLE_MAX_CAPACITY / 2) return SYMTABLE_ERR_ALLOC;
    int new_capacity = next_prime((symtable->capacity) * 2);
    item_t **new_items = calloc(new_capacity, sizeof(item_t *));
    if (new_items == NULL) return SYMTABLE_ERR_ALLOC;

    for (int i = 0; i < symtable->capacity; i++) {
        item_t *old = symtable->items[i];
        if (old == NULL) continue;

        unsigned int index = hash_bkdr(old->key) % new_capacity;

        // hladam nove pollicko dokym nenajdem nekolidujuce
        while (new_items[index] != NULL) {
            index = (index + 1) % new_capacity;
        }
        new_items[index] = old;
    }

    free(symtable->items);
    symtable->items = new_items;
    symtable->capacity = new_capacity;
    return SYMTABLE_OK;
}

symtable_status_t insert_data(symtable_t *symtable, const char *key, symtable_data_t data) {
    if (key == NULL || symtable == NULL) return SYMTABLE_ERR_ARGUMENT;
    if (search_item(symtable, key) != NULL) return SYMTABLE_ERR_EXISTS;

    if ((symtable->count + 1) * 10 > symtable->capacity * 7) {
        symtable_status_t status = resize_symtable(symtable);
        if (status != SYMTABLE_OK) return status;
    }

    unsigned int index = hash_bkdr(key) % symtable->capacity;

    for (int i = 0; i < symtable->capacity; i++) {
        item_t *item = symtable->items[index];

        // pre prazdne miesto vytvorim item
        if (item == NULL) {
            item = malloc(sizeof(item_t));
            if (item == NULL) return SYMTABLE_ERR_ALLOC;
            size_t len = strlen(key) + 1;
            item->key = malloc(len);
            if (item->key == NULL) {
                free(item);
                return SYMTABLE_ERR_ALLOC;
            }
            memcpy(item->key, key, len);
            item->data = data;
            symtable->items[index] = item;
            symtable->count++;
            return SYMTABLE_OK;
        }

        index = (index + 1) % symtable->capacity;
    }

    // tabulka je plna
    return SYMTABLE_ERR_FULL;
}

symtable_data_t *search_item(symtable_t *symtable, const char *key) {
    if ((symtable == NULL) || (key == NULL)) return NULL;
    unsigned int index = hash_bkdr(key) % symtable->capacity;

    for (int i = 0; i < symtable->capacity; i++) {
        item_t *item = symtable->items[index];

        if (item == NULL) return NULL;

        if (strcmp(item->key, key) == 0) return &item->data;

        index = (index + 1) % symtable->capacity;
    }
    return NULL;
}

void free_symtable(symtable_t *symtable) {
    if (symtable == NULL) return;

    for (int i = 0; i < symtable->capacity; i++) {
        if (symtable->items[i] == NULL) continue;
        free(symtable->items[i]->key);
        item_t *item = symtable->items[i];
        if (item->data.kind == SYMBOL_FUNCTION) {
            for (int j = 0; j < item->data.func.param_count; j++) {
                free(item->data.func.params[j].name);
            }
            free(item->data.func.params);
        }
        free(symtable->items[i]);
    }

    free(symtable->items);
    free(symtable);
}