/*
@author: Simona Studená (xstudes00)
@file: parser.c
*/
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "parser.h"


static FILE *current_file;
static Token current_token;

/*
@brief: function reading the advance token
*/
static void advance() {
    current_token = get_token(current_file);
}

static void syntax_error(const char *message) {
    fprintf(stderr, "Syntax error: %s\n", message);
    exit(2);
}

static ASTNode* parse_func_def(void);
//static ASTNode* parse_expression(void);
static ASTNode* parse_parameters(void);

ASTNode* parse(FILE *source_file) {
    current_file = source_file;
    advance();

    ASTNode* root = ast_create_node(NODE_PROGRAM);

    while (current_token.Ttype != TOKEN_EOF) {
        if(current_token.Ttype == TOKEN_EOL) {
            advance();
            continue;
        }
        if (current_token.Ttype == TOKEN_KW_DEF) {
            ASTNode *func = parse_func_def();
            (void)func;
        } else {
            syntax_error("missing key word def.\n");
        }
    }
    return root;
}

static ASTNode* parse_func_def(void) {
    advance();
    if(current_token.Ttype != TOKEN_IDENTIFIER) {
        syntax_error("missing or incorrect identifier.\n");
    }
    char *func_name = current_token.Tatribut.string;
    advance();

    if(current_token.Ttype != TOKEN_LPAREN) {
        syntax_error("missing left parenthesis.\n");
    }
    advance();

    ASTNode *params = parse_parameters();

    if(current_token.Ttype != TOKEN_RPAREN) {
        syntax_error("missing right parenthesis.\n");
    }
    advance();

    ASTNode *node = ast_create_node(NODE_FUNC_DEF);
    node->name = func_name;
    node->left = params;

    return node;
}

/*
@brief: function parsing parameters
*/
static ASTNode* parse_parameters(void) {
    ASTNode *first_param = NULL;
    ASTNode *last_param = NULL;

    if(current_token.Ttype == TOKEN_RPAREN) {
        return NULL;
    }

    while(1) {
        if(current_token.Ttype != TOKEN_IDENTIFIER) {
            syntax_error("missing or invalid parameter identifier.\n");
        }
        char *param_name = current_token.Tatribut.string;
        advance();

        if(current_token.Ttype != TOKEN_COLON) {
            syntax_error("missing colon.\n");
        }
        advance();

        if(current_token.Ttype != TOKEN_KW_INT && current_token.Ttype != TOKEN_KW_STRING && current_token.Ttype != TOKEN_KW_DOUBLE && current_token.Ttype != TOKEN_KW_UNIT){
            syntax_error("invalid Ttype of parameter.\n");
        }
        char *param_Ttype = current_token.Tatribut.string;
        advance();

        ASTNode *param_node = ast_create_node(NODE_PARAM);
        param_node->name = param_name;
        param_node->data_type = param_Ttype;
        param_node->right = NULL;

        if(first_param == NULL) {
            first_param = param_node;
        } else {
            last_param->right = param_node;
        }
        last_param = param_node;

        if(current_token.Ttype == TOKEN_COMMA) {
            advance();
            continue;
        } else {
            break;
        }
    }    

    return first_param;
}