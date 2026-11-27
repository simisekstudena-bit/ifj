/*
@author Simona Studená (xstudes00)
@file parser.h
*/

#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include <stdio.h>

ASTNode *parse_program(FILE *source_file);

#endif