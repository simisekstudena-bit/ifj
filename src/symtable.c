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
};

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

symtable_t *create_table(int size){
    if (size <= 0) return NULL;
    symtable_t *symtable = malloc(sizeof(*symtable));
    if (symtable == NULL) return NULL;

    symtable->items = calloc(size, sizeof(item_t));
    if (symtable->items == NULL) {
        free(symtable);
        return NULL;
    }

    symtable->capacity = size;
    return symtable;
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