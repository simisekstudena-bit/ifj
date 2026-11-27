/*
Projekt: Implementace prekladace
author: Jakub Ferencik xferej00
        Simona Studena xstudes00

Vstupny bod prekladaca
*/
#include "ast.h"
#include "errors.h"
#include "generator.h"
#include "parser.h"
#include "semantic.h"
#include "symtable.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    ASTNode *root = parse_program(stdin);

    int error = semantic_analysis(root);
    if (error != ERROR_OK) {
        ast_free(root);
        return error;
    }

    gen_init();
    gen_program(root);
    gen_print(stdout);
    gen_free();

    ast_free(root);
    return ERROR_OK;
}
