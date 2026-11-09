/*
Projekt: Implementace prekladace
author: Jakub Ferencik xferej00

File obsahuje zdielany enumdatovych typov ifj26 pre parser a symtable
*/

#ifndef TYPES_H
#define TYPES_H

typedef enum {
    TYPE_NONE = 0, // type je zatial neodvodeny
    TYPE_INT,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_UNIT,
    TYPE_ANY, // parametre funkcii
} DataType;

#endif