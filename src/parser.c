/*
@author Simona Studená (xstudes00)
@file parser.c
*/
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "parser.h"


static FILE *current_file;
static Token current_token;

/*
@brief function reading the advance token
*/
static void advance() {
    current_token = get_token(current_file);
}

/*
@brief prints message to stderrin case of error and exits with exit code 2
@param message message to write to stderr
*/
static void syntax_error(const char *message) {
    fprintf(stderr, "Syntax error: %s\n", message);
    exit(2);
}

/*
@brief expects token type then steps over
@param type expected token type
@param message message to write to stderr if type not met
*/
static void expect(TokenType type, const char *message) {
    if (current_token.Ttype != type) {
        syntax_error(message);
    }
    advance();
}

/*
@brief expects token type but doesnt step over
@param type expected token type
@param message message to write to stderr if type not met
*/
static void expect_no_step(TokenType type, const char *message) {
    if (current_token.Ttype != type) {
        syntax_error(message);
    }
}

/*
@brief skips end of line tokens
*/
static void skip_eols(void){
    while (current_token.Ttype == TOKEN_EOL) {
        advance();
    }
}

/*
@brief converts type key word to DataType
@param type type key word to be converted
@return converted type kw in DataType
*/
static DataType token_to_type(TokenType type) {
    switch (type) {
        case TOKEN_KW_INT:      return TYPE_INT;
        case TOKEN_KW_DOUBLE:   return TYPE_DOUBLE;
        case TOKEN_KW_STRING:   return TYPE_STRING;
        case TOKEN_KW_UNIT:     return TYPE_UNIT;
        default:                return TYPE_NONE;
    }
}

/*
@brief parses <type>
@return type
*/
static DataType parse_type(void){
    DataType type = token_to_type(current_token.Ttype);
    if(type == TYPE_NONE) {
        syntax_error("missing or invalid type.");
    }
    advance();
    return type;
}

/*
@brief parses <opt_type> → : <type> | epsilon
*/
static DataType parse_opt_type(void){
    if(current_token.Ttype == TOKEN_COLON) {
        advance();
        return parse_type();
    }
    return TYPE_NONE;
}


static ASTNode* parse_func_def(void);
static ASTNode* parse_parameters(void);
static ASTNode *parse_func_body(void);
static ASTNode *parse_block(void);
static ASTNode *parse_statement(void);
static ASTNode *parse_expression(void);

//TODO: spravit to pozirani prologu az bude scanner znat prolog token
static void parse_prolog(void) {
    expect(TOKEN_KW_IMPORT, "missing prolog 'import ifj26.*'.");
    if(current_token.Ttype != TOKEN_IDENTIFIER || strcmp(current_token.Tatribut.string, "ifj26") != 0) {
        syntax_error("incorrect prolog.");
    }
    free(current_token.Tatribut.string);
    advance();
    if(strcmp(current_token.Tatribut.string, ".") != 0) {
        syntax_error("expected '.' in prolog");
    }
    expect(TOKEN_MUL, "expected '*' in prolog.");
    expect(TOKEN_EOL, "expected end of line after prolog.");
}

ASTNode *parse_program(FILE *source_file) {
    current_file = source_file;
    advance();
    skip_eols();

    parse_prolog();
    skip_eols();

    ASTNode *root = ast_create_node(NODE_PROGRAM);

    while (current_token.Ttype != TOKEN_EOF) {
        root->body = ast_append(root->body, parse_func_def());
        skip_eols();
    }
    return root;
}

/*
@brief parses function definition
*/
static ASTNode* parse_func_def(void) {
    bool is_main = false;
    //<opt_main>
    //TODO: opravit, az bude @ token
    if(current_token.Ttype == TOKEN_IDENTIFIER && current_token.Tatribut.string != NULL && strcmp(current_token.Tatribut.string, "@main") == 0) {
        is_main = true;
    }

    expect(TOKEN_KW_DEF, "expected key word 'def'.");

    expect_no_step(TOKEN_IDENTIFIER, "missing or invalid function name.");
    ASTNode *node = ast_create_node(NODE_FUNC_DEF);
    node->name = current_token.Tatribut.string;
    node->is_main = is_main;
    advance();

    expect(TOKEN_LPAREN, "missing '(' in function declaration.");
    node->params = parse_parameters();
    expect(TOKEN_RPAREN, "missing ')' in function declaration.");

    node->data_type = parse_opt_type();
    expect(TOKEN_ASSIGN, "missing '=' in function declaration.");

    node->body = parse_func_body();

    return node;
}

/*
@brief: function parsing parameters
*/
static ASTNode* parse_parameters(void) {
    ASTNode *head = NULL;
    ASTNode *tail = NULL;

    while(current_token.Ttype != TOKEN_RPAREN) {
        expect_no_step(TOKEN_IDENTIFIER, "missing or invalid parameter name.");
        ASTNode *param = ast_create_node(NODE_PARAM);
        param->name = current_token.Tatribut.string;
        advance();

        expect(TOKEN_COLON, "missing ':' after parameter name.");
        param->data_type = parse_type();

        if(head == NULL) {
            head = param;
        } else {
            tail->next = param;
        }
        tail = param;
        if(current_token.Ttype == TOKEN_COMMA) {
            advance();
        } else {
            expect_no_step(TOKEN_RPAREN, "expected ',' or ')' in parameter list.");
        }
    }
    return head;
}

/*
@brief parsing function body
*/
static ASTNode *parse_func_body(void) {
    if (current_token.Ttype == TOKEN_EOL) {
        return parse_block();
    }
    ASTNode *expr = parse_expression();
    expect(TOKEN_EOL, "expected end of line after function body.");
    return expr;
}

static ASTNode *parse_block(void) {
    expect(TOKEN_EOL, "expected end of line before block.");
    expect(TOKEN_INDENT, "expected indented block.");

    ASTNode *block = ast_create_node(NODE_BLOCK);

    do {
        block->body = ast_append(block->body, parse_statement());
    } while(current_token.Ttype != TOKEN_DEDENT);

    expect(TOKEN_DEDENT, "expected dedent after end of block.");
    return block;
}

//TODO doplnit funkce pro statement a expression

static ASTNode *parse_statement(void) {
    syntax_error("statements are not implemented yet.");
    return NULL;
}

static ASTNode *parse_expression(void) {
    syntax_error("expressions are not implemented yet.");
    return NULL;
}