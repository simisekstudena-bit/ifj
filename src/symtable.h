/*
Projekt: Implementace prekladace
author: Jakub Ferencik xferej00

File obsahuje rozhranie tabulky symbolov 
*/

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdbool.h>

/**
 * Navratove stavy tabulky symbolov. 
 * Mapovanie robi volajuci.
 */
typedef enum{
    SYMTABLE_OK = 0,
    SYMTABLE_ERR_ALLOC, // nepodarilo sa alokovat polozku
    SYMTABLE_ERR_EXISTS, // navratovy stav pre pripad ze prvok s rovnakym 
                         // klucom existuje. treba najst prvok a manualne ho prepisat
                         // ak je prepisanie potrebne
    SYMTABLE_ERR_FULL, // symtable je plny (todo: preratavat velkost? slebo buffer)
    SYMTABLE_ERR_ARGUMENT, // NULL
} symtable_status_t;

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

typedef struct symtable symtable_t;

typedef struct block {
    symtable_t *symtable;
    struct block *parent;
} block_t;

typedef struct {
    int size;
    block_t *top;
} symtable_stack_t;


typedef char *symtable_key_t;

typedef struct {
    symtable_data_t data;
    symtable_key_t key;
} item_t;

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

/**
 * Funkcia vlozi do tabulky symbol.
 * Ak kluc existuje funkcia vrati ze sa v tabulke nachadza policko s rovnakym klucom.
 * a treba manualne prepisa5 policko
 * @param symtable tabulka symbolov v ktorej chcem prvok vytvorit
 * @param key meno symbolu ktory ukladam do tabulky
 * @param data informacie o vkladanom symbole
 * 
 * @return stavy z symtable_status_t o tom ci presiel zapis
 */
symtable_status_t insert_data(symtable_t *symtable, symtable_key_t key,symtable_data_t data);

/**
 * Funkcia vyhlada podla kluca polozku s rovnakym klucom v tabulke
 * 
 * @param symtable tabulka symbolov v ktorej chceme prvok hladat
 * @param key nazov ktory chceme v tabulke vyhladat
 * 
 * @return item ktory ma takyto key, ak neexistuje/nenasli sme vrati NULL 
 */
item_t *search_item(symtable_t *symtable, symtable_key_t key);
#endif
