#ifndef ERRORS_H
#define ERRORS_H

typedef enum {
    ERROR_OK = 0,
    LEXICAL_ERR = 1,
    SYNTACTIC_ERR = 2,
    SEM_VAL_UNDEFINED_ERR = 3,   // Pouzitie premennej pred jej deklaraciou
    SEM_REDEF_ERR = 4,           // Opakovana definicia v rovnakom bloku, redefinicia
    SEM_CALL_ERR = 5,            // Zly pocet/typ parametrov.
    SEM_DIFF_TYPE_ERR = 6,       // Pokus o priradenie hodnoty ineho typu
    SEM_INITIALIZATION_ERR = 7,  // Zapis do premnnej ktora sa uz neda menit
    SEM_OTHER_ERROR = 10,        // ostatne semanticke chyby
    INTERNAL_INTERPRET_ERR = 99, // vsetko ostnatne mainly alokacne chyby
} errors;

#endif