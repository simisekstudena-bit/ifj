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
 * @param str kluc ukonceny "\0"
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

bool insert_data(symtable_t *symtable, symtable_key_t key,symtable_data_t data){
    if (key == NULL || symtable == NULL) return false;

    unsigned int index = hash_bkdr(key) % symtable->capacity;

    for (int i = 0; i < symtable->capacity; i++){
        item_t *item = &symtable->items[index];

        // pre prazdne miesto vytvorim item
        if (item->key == NULL){
            int len = strlen(key);
            item->key = malloc(len);
            if (item->key == NULL) return false;
            memcpy(item->key, key, len);
        }

        // aktualizacna semantika
        if(strcmp(item->key, key) == 0) {
            item->data = data;
            return true;
        }
        
        // posuniem sa na dalsiu polozku kvoli kolizii s rozdielnymi datami na rovnakom iteme
        index = index + 1 % symtable->capacity;
    }

    // tabulka je plna
    return false; 
}

void free_symtable(symtable_t *symtable){
    if (symtable == NULL) return;

    free(symtable->items);
    free(symtable);
}