/*
@author Simona Studená (xstudes00)
@file parser.h
*/

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "ast.h"

ASTNode* parse_program(FILE *source_file);

#endif