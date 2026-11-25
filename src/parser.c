/*
@author Simona Studená (xstudes00)
@file parser.c
*/
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "parser.h"
#include "parser_internal.h"
#include "expression.h"


static FILE *current_file;
Token current_token;
/*
in case of needed lookahead, LL(2)
*/
static Token lookahead;
static bool has_lookahead = false;

/*
@brief function reading the advance token
*/
void advance() {
    if (has_lookahead) {
        current_token = lookahead;
        has_lookahead = false;
    } else {
    current_token = get_token(current_file);
    }
}

/*
@brief peeks for the type of the next token when lookahead is needed
*/
TokenType peek_type(void) {
    if(!has_lookahead) {
        lookahead = get_token(current_file);
        has_lookahead = true;
    }
    return lookahead.Ttype;
}
/*
@brief prints message to stderrin case of error and exits with exit code 2
@param message message to write to stderr
*/
void syntax_error(const char *message) {
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
@return either the type or none when the type is not specified
*/
static DataType parse_opt_type(void){
    if(current_token.Ttype == TOKEN_COLON) {
        advance();
        return parse_type();
    }
    return TYPE_NONE;
}

/*
forward declaration of used functions
*/
static ASTNode *parse_func_def(void);
static ASTNode *parse_parameters(void);
static ASTNode *parse_func_body(void);
static ASTNode *parse_block(void);
static ASTNode *parse_statement(void);
static ASTNode *parse_var_decl(void);
static ASTNode *parse_return(void);
static ASTNode *parse_assign(void);
static ASTNode *parse_if(void);
static ASTNode *parse_while(void);
static ASTNode *parse_arguments(void);
//static ASTNode *parse_term(void);

//TODO: spravit to pozirani prologu az bude scanner znat prolog token
static void parse_prolog(void) {
    expect(TOKEN_KW_IMPORT, "missing prolog 'import ifj26.*'.");
    expect(TOKEN_IFJ26, "missing ifj26 in prolog.");
    expect(TOKEN_DOT, "missing '.' in prolog");
    expect(TOKEN_MUL, "expected '*' in prolog.");
    expect(TOKEN_EOL, "expected end of line after prolog.");
}

/*
@brief parses program following the grammar rule <program> → import ifj26.* EOL <func_list>
*/
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
@return node of the function definition
*/
static ASTNode* parse_func_def(void) {
    bool is_main = false;
    //<opt_main>
    //TODO: opravit, az bude @ token
    if(current_token.Ttype == TOKEN_AT_MAIN) {
        is_main = true;
        advance();
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
@brief: parses parameters in function declaration
@return head the firts parameter

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

/*
@brief parses block
*/
static ASTNode *parse_block(void) {
    expect(TOKEN_EOL, "expected end of line before block.");
    expect(TOKEN_INDENT, "expected indented block.");

    ASTNode *block = ast_create_node(NODE_BLOCK);

    while (current_token.Ttype != TOKEN_DEDENT && current_token.Ttype != TOKEN_EOF){
        block->body = ast_append(block->body, parse_statement());
    }
    if(current_token.Ttype == TOKEN_DEDENT) {
        advance();
    } else {
        syntax_error("expected dedent after end of block or end of file.");
    }
    return block;
}

/*
@brief parses statements based on their keywords
*/
static ASTNode *parse_statement(void) {
    ASTNode *node;

    if (current_token.Ttype == TOKEN_IDENTIFIER && peek_type() == TOKEN_ASSIGN) {
        node = parse_assign();
        expect(TOKEN_EOL, "expected end of line after assing");
        return node;
    }
    switch (current_token.Ttype) {
        case TOKEN_KW_IF:
            return parse_if();
        case TOKEN_KW_WHILE:
            return parse_while();
        case TOKEN_KW_VAL:
        case TOKEN_KW_VAR:
            node = parse_var_decl();
            break;
        case TOKEN_KW_RETURN:
            node = parse_return();
            break;
        default:
            node = ast_create_node(NODE_EXPR_STMT);
            node->left = parse_expression();
            break;
    }
    expect(TOKEN_EOL, "expected end of line after statement.");
    return node;
}

/*
@brief parses assign statement
*/
static ASTNode *parse_assign(void) {
    ASTNode *node = ast_create_node(NODE_ASSIGN);
    node->name = current_token.Tatribut.string;
    advance();
    expect(TOKEN_ASSIGN, "expected '=' in assign statement.");
    node->left = parse_expression();
    return node;
}

/*
@brief parses if statement
*/
static ASTNode *parse_if(void) {
    ASTNode *node = ast_create_node(NODE_IF);
    advance();
    node->cond = parse_expression();
    expect(TOKEN_KW_THEN, "expected 'then' in if statement.");
    node->body = parse_block();
    expect(TOKEN_KW_ELSE, "expected 'else' in if statement.");
    node->else_body = parse_block();
    return node;
}

/*
@brief parses while statement
*/
static ASTNode *parse_while(void) {
    ASTNode *node = ast_create_node(NODE_WHILE);
    advance();
    node->cond = parse_expression();
    expect(TOKEN_KW_DO, "expected 'do' in while statement.");
    node->body = parse_block();
    return node;
}

/*
@brief parses return statement
*/
static ASTNode *parse_return(void) {
    expect(TOKEN_KW_RETURN, "expected keyword 'return'.");
    ASTNode *node = ast_create_node(NODE_RETURN);
    if(current_token.Ttype != TOKEN_EOL) {
        node->left = parse_expression();
    }
    return node;
}

/*
@brief parses variable declaration
whether is it a var or val is noted in is_var in the node
*/
static ASTNode *parse_var_decl(void) {
    bool is_var = (current_token.Ttype == TOKEN_KW_VAR);

    if (current_token.Ttype == TOKEN_KW_VAL || current_token.Ttype == TOKEN_KW_VAR) {
        advance();
    } else {
        syntax_error("expected 'val' or 'var' in variable declaration.");
    }

    expect_no_step(TOKEN_IDENTIFIER, "missing or invalid variable name");
    ASTNode *node = ast_create_node(NODE_VAR_DECL);
    node->is_var = is_var;
    node->name = current_token.Tatribut.string;
    advance();

    node->data_type = parse_opt_type();
    expect(TOKEN_ASSIGN, "missing '=' in variable declaration");
    node->left = parse_expression();

    return node;
}

/*
@brief parses function call
*/
ASTNode *parse_func_call(void){
    expect_no_step(TOKEN_IDENTIFIER, "expected function name.");
    ASTNode *call = ast_create_node(NODE_FUNC_CALL);
    call->name = current_token.Tatribut.string;
    advance();
    expect(TOKEN_LPAREN, "expected '(' in function call");
    call->params = parse_arguments();
    expect(TOKEN_RPAREN, "expected ')' in function call.");
    return call;
}

/*
@brief parses arguments of function call
*/
static ASTNode *parse_arguments(void){
    ASTNode *head = NULL;
    while (current_token.Ttype != TOKEN_RPAREN) {
        head = ast_append(head, parse_expression());
        if(current_token.Ttype == TOKEN_COMMA) {
            advance();
        } else {
            expect_no_step(TOKEN_RPAREN, "expected ',' or ')' in arguments.");
        }
    }
    return head;
}

/*
@brief parses term

static ASTNode *parse_term(void) {
    ASTNode *node;
    switch (current_token.Ttype) {
        case TOKEN_LITERAL_INT:
            node = ast_create_node(NODE_INT_LITERAL);
            node->int_value = current_token.Tatribut.in;
            break;
        case TOKEN_LITERAL_DOUBLE:
            node = ast_create_node(NODE_DOUBLE_LITERAL);
            node->double_value = current_token.Tatribut.db;
            break;
        case TOKEN_LITERAL_STRING:
            node = ast_create_node(NODE_STRING_LITERAL);
            node->string_value = current_token.Tatribut.string;
            break;
        case TOKEN_LITERAL_UNIT:
            node = ast_create_node(NODE_UNIT_LITERAL);
            break;
        case TOKEN_IDENTIFIER:
            node = ast_create_node(NODE_IDENTIFIER);
            node->name = current_token.Tatribut.string;
            break;
        default:
            syntax_error("expected literal or identifier");
            return NULL;
    }
    advance();
    return node;
}
*/