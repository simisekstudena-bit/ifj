/*
Projekt: Implementace prekladace
author: Jakub Ferencik xferej00

File obsahuje source code tabulky symbolov
*/

#include "symtable.h"
#include <stdlib.h>

struct symtable {
    polozka_t *items;
    int capacity;
};

symtable_t *create_table(int size){
    symtable_t *symtable = malloc(sizeof(*symtable));
    if (symtable == NULL) return NULL;

    symtable->items = calloc(size, sizeof(polozka_t));
    if (symtable->items == NULL) {
        free(symtable);
        return NULL;
    }

    symtable->capacity = size;
    return symtable;
}

void free_symtable(symtable_t *symtable){
    if (symtable == NULL) return;

    free(symtable->items);
    free(symtable);
}