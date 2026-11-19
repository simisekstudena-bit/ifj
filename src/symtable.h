/*
Projekt: Implementace prekladace
author: Jakub Ferencik xferej00

File obsahuje rozhranie tabulky symbolov 
*/

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdbool.h>
#include "types.h"
struct ASTNode;

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
    SYMTABLE_ERR_FULL,   //tento stav BY NEMAL nastat ale stale sa tu nachadza
                         // aby som nemal nekonecny cyklus
    SYMTABLE_ERR_ARGUMENT, // NULL
} symtable_status_t;

/**
 * Druh symbolu
 */
typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_PARAMETER,
    SYMBOL_FUNCTION
} symtable_kind_t;


// parameter funkcie

typedef struct {
    char *name;
    DataType type;
} symtable_param_t;


// data pre premennu a parameter

typedef struct {
    bool is_modifiable; // var = true, val = false
    int scope_id;    // jedinecne cislo rozsahu pre generator
} symtable_var_t;


 // data pre funkciu

typedef struct {
    symtable_param_t *params; // dynamicke pole parametrov, plni sa cez add_parameter
    int param_count;
    int param_capacity;
    bool return_explicit; // navratovy typ bol uvedeny
    bool is_main;
    bool builtin;         // vstavana funkcia
    bool inferring;       // navratovy typ sa prave odvodzuje - volanie seba
    struct ASTNode *node; // ukazatel na node s ktoreho bola vytvorena
    bool checked;         // ci bola skontrolovana semantikou
} symtable_func_t;

typedef struct {
    symtable_kind_t kind;
    DataType data_type; // typ premennej/parametra, pri funkcii navratovy typ
    union {
        symtable_var_t var;   
        symtable_func_t func; 
    };
} symtable_data_t;

typedef struct symtable symtable_t;
typedef struct symtable_stack symtable_stack_t;

/**
 * Funkcia vytvori prazdnu tabulku symbolov
 * Kapacita s azaokruhluje na najblizsie prvocislo
 * 
 * @param size mnozstvo poloziek v zozname
 * @return ukazatel na symtable, NULL pri chybe alokacie
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
 * @return stavy z symtable_status_t o tom ci presiel zapis.
 *         insert vie taktiez nafukovat pole, pointre vratene zo search_item ale ostavaju platne
 */
symtable_status_t insert_data(symtable_t *symtable, const char *key,symtable_data_t data);

/**
 * Funkcia vyhlada podla kluca polozku s rovnakym klucom v tabulke
 * 
 * @param symtable tabulka symbolov v ktorej chceme prvok hladat
 * @param key nazov ktory chceme v tabulke vyhladat
 * 
 * @return data itemu s tymto klucom NULL ak neexistuje
 */
symtable_data_t *search_item(symtable_t *symtable, const char *key);

/**
 * Funkcia inicializuje prazdny stack
 * @return pointer na inicializovany stack, NULL pri chybe alokacie
 */
symtable_stack_t *init_stack(void);

/**
 * Skontroluje ci je stack prazdny
 * @param stack ktory chceme skontrolovat
 * @return true ak je, false ak nie
 */
bool stack_is_empty(symtable_stack_t *stack);

/**
 * Funkcia pouzivatelovi vrati vrchny symtable zo stacku
 * @param stack ktory chceme skontrolovat
 * @return pointer na symtable na vrchu stacku. Symtable patri stacku, volajuci ho nesmie
 *         uvolnit a po stack_pop/free_stack pointer prestava platit
 */
symtable_t *stack_top(symtable_stack_t *stack);

/**
 * Funkia vlozi zadany symtable na vrch stacku
 * @param stack ktory na ktory pushujeme
 * @param symtable ktory vkladame na vrch stacku
 * @return SYMTABLE_OK, potom symtable patri stacku a uvolni ho stack_pop/free_stack.
 *         pri chybe (SYMTABLE_ERR_ALLOC, SYMTABLE_ERR_ARGUMENT) sa stack nezmeni
 *         a symtable musi uvolnit volajuci.
 *         po uspesnom pushi volajuci symtable sam neuvolnuje a ten isty symtable
 *         sa nesmie pushnut dvakrat (vznikne dva krat pointer na rovnaky symtable)
 */
symtable_status_t stack_push(symtable_stack_t *stack, symtable_t *symtable);

/**
 * Funkcia vyhodi a uvolni symtable (blok) z vrchu stacku. symtable sa uvolni cez free_symtable
 *  aj so vsetkymi polozkami, takze pointre zo stack_top,
 * search_item a stack_search do tohto symtablu prestanu platit
 * @param stack z ktoreho chceme vrchny prvok vybrat
 */
void stack_pop(symtable_stack_t *stack);

/**
 * Funkcia prehlada vsetky symtables v stacku pre dany key
 * @param stack stack v ktorom chceme prvok najst
 * @param key kluc ktory hladam
 * @return pointer na data_t s dannym klucom, null ak kluc nenajde. Ak pouzivatel
 *         spravi stack_pop tak pointer vrateny stack_search platit prestane
 */
symtable_data_t *stack_search(symtable_stack_t *stack, const char *key);

/**
 * Funkcia uvolni vsetko zo stacku, vratane vsetkych symtablov ktore su v nom
 * @param stack ktory chceme cely uvolnit
 */
void free_stack(symtable_stack_t *stack);

/**
 * Funkcia prida funkcii parameter na koniec zoznamu jej parametrov, meno sa skopiruje.
 * Volat raz pre kazdy parameter v poradi ako su v definicii
 * @param data data funkcie zo search search_item
 * @param name meno parametra
 * @param type typ parametra
 * @return SYMTABLE_OK
 *         SYMTABLE_ERR_EXISTS ak funkcia uz ma parameter s takym menom (chyba 4)
 *         SYMTABLE_ERR_ARGUMENT ak je data alebo name NULL, alebo data nie je funkcia
 *         SYMTABLE_ERR_ALLOC pri chybe alokacie
 */
symtable_status_t add_parameter(symtable_data_t *data, const char *name, DataType type);

/**
 * Funkcia ktora volatelovi najde id vrchneho itemu/bloku
 * @param stack na ktorom chceme id najst
 * @return jedinecne cislo rozsahu na vrchu itemu/bloku, -1 ak je symtable NULL a aj pri prazdnom zasobniku
 */
int stack_scope_id(symtable_stack_t *stack);
#endif