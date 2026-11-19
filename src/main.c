/*
Projekt: Implementace prekladace
author: Jakub Ferencik xferej00

Vstupny bod prekladaca
*/
#include <stdio.h>
#include <stdlib.h>
#include "symtable.h"
#include "parser.h"
#include "ast.h"

int main(void)
{
    ASTNode *root = parse_program(stdin);

    ast_free(root);
    
    return 0;
}
