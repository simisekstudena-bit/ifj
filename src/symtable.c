/*
Projekt: Implementace prekladace
author: Jakub Ferencik xferej00

File obsahuje source code tabulky symbolov
*/

#include "symtable.h"
#include <stdlib.h>
#include <string.h>

struct symtable {
    item_t *items;
    int capacity;
    int count;
};

symtable_stack_t *init_stack(void){
    symtable_stack_t *stack = malloc(sizeof(symtable_stack_t));
    if (stack == NULL) return NULL;
    stack->top = NULL;
    stack->size = 0;
    return stack;
}

symtable_t *stack_top(symtable_stack_t *stack){
    if (stack->top == NULL) return NULL;
    if (stack->top != NULL) return stack->top->symtable;
    else return NULL;
}

symtable_stack_t *stack_push(symtable_stack_t *stack, symtable_t *symtable){
    block_t *block = malloc(sizeof(block_t));
    if (block == NULL) return NULL;

    block->symtable = symtable;
    block->parent = stack->top;
    stack->top = block;
    stack->size++;
    return stack;
}

symtable_stack_t *stack_pop(symtable_stack_t *stack) {
    if (stack == NULL || stack->top == NULL) return NULL;
    block_t *top_block = stack->top;
    stack->top = top_block->parent;
    stack->size--;
    
    free_symtable(top_block->symtable);
    free(top_block);
    return stack;
}

/**
 * Mapovacia funkcia BKDR, vypocita hash z retazca kluca
 * Algoritmus som zobral z prednaskovych slajdov IAL pre minuly rok
 *
 * @param str kluc ukonceny '\0'
 * @return hash kluca, index do tabulky je hash % capacity
 */
static unsigned int hash_bkdr(const char *str){
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
static bool is_prime(int n){
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
static int next_prime(int n){
    if (n < 2) return 2;

    while (!is_prime(n)) {
        n++;
    }

    return n;
}

symtable_t *create_table(int size){
    if (size <= 0) return NULL;
    symtable_t *symtable = malloc(sizeof(*symtable));
    if (symtable == NULL) return NULL;
    
    symtable->capacity = next_prime(size);
    symtable->items = calloc(size, sizeof(item_t));
    if (symtable->items == NULL) {
        free(symtable);
        return NULL;
    }

    return symtable;
}

/**
 * Zvacsime tabulku na najblizsie vacsie prvocislo a potom presunieme prvky tabulky na ich nove
 * prepocitane indexy. 
 * 
 * @param symtable tabulka ktoru zvacsujem
 * @return SYMTABLE_OK alebo chybovy stav, nove polozky aj s pridanym miesotm zostavaju
 *         pod rovnakou tabulkou
 */
static symtable_status_t *resize_symtable(symtable_t *symtable) {
    int new_capacity = next_prime((symtable->capacity) * 2);
    item_t *new_items = calloc(new_capacity, sizeof(item_t));
    if (new_items == NULL) return SYMTABLE_ERR_ALLOC;

    for (int i = 0; i < symtable->capacity; i++){
        item_t *old = &symtable->items[i];
        if (old->key != NULL) continue;
        
        unsigned int index = hash_bkdr(old->key) % new_capacity;
        
        // hladam nove pollicko dokym nenajdem nekolidujuce
        while (new_items[index].key != NULL) {
            index = (index + 1) % new_capacity;
        }
        new_items[index] = *old;
    }
    
    free(symtable->items);
    symtable->items = new_items;
    symtable->capacity = new_capacity;
    return SYMTABLE_OK;
}

symtable_status_t insert_data(symtable_t *symtable, symtable_key_t key,symtable_data_t data){
    if (key == NULL || symtable == NULL) return SYMTABLE_ERR_ARGUMENT;

    unsigned int index = hash_bkdr(key) % symtable->capacity;

    for (int i = 0; i < symtable->capacity; i++){
        item_t *item = &symtable->items[index];

        // pre prazdne miesto vytvorim item
        if (item->key == NULL){
            size_t len = strlen(key) + 1;
            item->key = malloc(len);
            if (item->key == NULL) return SYMTABLE_ERR_ALLOC;
            memcpy(item->key, key, len);
            item->data = data;
            return SYMTABLE_OK;
        }

        // vracia ze zapis s rovnakym klucom existuje
        if(strcmp(item->key, key) == 0) {
            return SYMTABLE_ERR_EXISTS;
        }
        
        // posuniem sa na dalsiu polozku kvoli kolizii s rozdielnymi datami na rovnakom iteme
        index = (index + 1) % symtable->capacity;
    }

    // tabulka je plna 
    return SYMTABLE_ERR_FULL; 
}

item_t *search_item(symtable_t *symtable, symtable_key_t key) {
    if ((symtable == NULL)||(key == NULL)) return NULL;
    unsigned int index = hash_bkdr(key) % symtable->capacity;

    for (int i = 0; i < symtable->capacity; i++){
        item_t *item = &symtable->items[index];

        if (item->key == NULL) return NULL;

        if (strcmp(item->key, key) == 0) return item;

        index = (index + 1) % symtable->capacity;
    }
    return NULL;
}

void free_symtable(symtable_t *symtable){
    if (symtable == NULL) return;

    for (int i = 0; i < symtable->capacity; i++) {
        free(symtable->items[i].key);
    }

    free(symtable->items);
    free(symtable);
}