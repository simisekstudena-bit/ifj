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
pokud bude potreba lookahead, LL(2)
*/
static Token lookahead;
static bool has_lookahead = false;

/*
@brief čte následující token
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
@brief vrací typ následujícího tokenu v případě lookahead
@return typ následujícího tokenu
*/
TokenType peek_type(void) {
    if(!has_lookahead) {
        lookahead = get_token(current_file);
        has_lookahead = true;
    }
    return lookahead.Ttype;
}
/*
@brief vypíše zprávu na stderr a ukončí s návratovým kódem 2
@param message zpráva k vypssání na stderr
*/
void syntax_error(const char *message) {
    fprintf(stderr, "Syntax error: %s\n", message);
    exit(2);
}

/*
@brief zkontroluje očekávaný datový typ a pokročí dál v proudu tokenů
@param očekávaný datový typ
@param message zpráva, kterou vypíše na stderr, pokud se typ neshoduje
*/
static void expect(TokenType type, const char *message) {
    if (current_token.Ttype != type) {
        syntax_error(message);
    }
    advance();
}

/*
@brief očekává token, ale nedělá krok
@param type eočekávaný typ
@param message zpráva, kterou vypíše na stderr, pokud se typ neshoduje
*/
static void expect_no_step(TokenType type, const char *message) {
    if (current_token.Ttype != type) {
        syntax_error(message);
    }
}

/*
@brief přeskakuje znaky eol
*/
static void skip_eols(void){
    while (current_token.Ttype == TOKEN_EOL) {
        advance();
    }
}

/*
@brief převádí typ klíčového slova na DataType
@param type klíčevé slovo datového typu, které má být převedeno
@return převedené klíčové slovo datového typu v datovém typu DataType
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
@brief parsuje <type>
@return datový typ
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
@brief parsuje <opt_type> → : <type> | epsilon
@return buďto typ uzlu, nebo nic, pokud nebyl datový typ specifikován
*/
static DataType parse_opt_type(void){
    if(current_token.Ttype == TOKEN_COLON) {
        advance();
        return parse_type();
    }
    return TYPE_NONE;
}

/*
dopředná deklarace funkcí
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

/*
@brief zpracuje prolog
*/
static void parse_prolog(void) {
    expect(TOKEN_KW_IMPORT, "missing prolog 'import ifj26.*'.");
    expect(TOKEN_IFJ26, "missing ifj26 in prolog.");
    expect(TOKEN_DOT, "missing '.' in prolog");
    expect(TOKEN_MUL, "expected '*' in prolog.");
    expect(TOKEN_EOL, "expected end of line after prolog.");
}

/*
@brief zpracuje program podle pravidla <program> → import ifj26.* EOL <func_list>
připojuje nové uzly do ast
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
@brief zpracovává definici funkcí
@return uzel definice funkce
*/
static ASTNode* parse_func_def(void) {
    bool is_main = false;
    //<opt_main>
    if(current_token.Ttype == TOKEN_AT_MAIN) {
        is_main = true;
        advance();
        while (current_token.Ttype == TOKEN_EOL) {
            advance();
        }
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
@brief zpracovává parametry deklarace funkce
@return head první parametr

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
@brief zpracovává tělo funkce
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
@brief zpracovává blok
*/
static ASTNode *parse_block(void) {
    expect(TOKEN_EOL, "expected end of line before block.");
    expect(TOKEN_INDENT, "expected indented block.");

    ASTNode *block = ast_create_node(NODE_BLOCK);

    while (current_token.Ttype != TOKEN_DEDENT && current_token.Ttype != TOKEN_EOF){
        block->body = ast_append(block->body, parse_statement());
        if (current_token.Ttype == TOKEN_SEMICOLON) {
            advance();
        } else if (current_token.Ttype == TOKEN_EOL) {
            advance();
        } else if (current_token.Ttype != TOKEN_DEDENT) {
            syntax_error("expected end of line or semicolon after statement.");
        }
    }
    if(current_token.Ttype == TOKEN_DEDENT) {
        advance();
    } else {
        syntax_error("expected dedent after end of block or end of file.");
    }
    return block;
}

/*
@brief zpracovává příkazy podle jejich klíčového slova
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
    return node;
}

/*
@brief zpracovává příkaz přiřazení
@return node uzel přiřazení
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
@brief zpracovává příkaz if
@return node uzel if
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
@brief zpracovává příkaz while
@return node uzel while
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
@brief zpracovává return
@return node uzel return
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
@brief zpracovává deklaraci proměnných
uvádí, jestli se jedná o var nebo val
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
@brief zpracovává volání funkce
@return uzel volání
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
@brief zpracovává argumenty volání funkce
@return head první argument
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