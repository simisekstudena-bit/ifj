/*
Projekt: Implementace prekladace
author: Jakub Ferencik xferej00

File obsahuje rozhranie tabulky symbolov 
*/

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdbool.h>

typedef enum {
    TYPE_INT,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_UNIT
} symtable_data_type_t;

typedef struct {
    symtable_data_type_t data_type;
    bool function; 
    bool variable;
}symtable_data_t;

typedef char *symtable_key_t;

typedef struct {
    symtable_data_t data;
    symtable_key_t key;
} polozka_t;

typedef struct symtable symtable_t;

/**
 * Funkcia vytvori prazdnu tabulku symbolov
 * 
 * @param size mnozstvo poloziek v zozname
 * @return ukazatel na symtable, na zatial NULL pri chybe alokacie
 */
symtable_t *create_table(int size);

/**
 * Funkcia uvolni tabulku prvkov a vsetky jej alokovane komponenty
 * 
 * @param symtable tabulka ktoru chceme uvolnit
 */
void free_symtable(symtable_t *symtable);

#endif
