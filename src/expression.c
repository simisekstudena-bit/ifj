/*
@author Simona Studená (xstudes00)
@file expression.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "expression.h"
#include "parser_internal.h"
#include "scanner.h"

/*
druhy prvků na zásobníku
*/
typedef enum {
    ITEM_TERMINAL,
    ITEM_NONTERMINAL,
    ITEM_HANDLE
} ItemKind;

/*
struktura prvku na zásobníku
*/
typedef struct {
    ItemKind kind;
    PrecSymbol sym;
    Token token;
    ASTNode *node;
} StackItem;

/*
struktura zásobníku
*/
typedef struct {
    StackItem *items;
    int size;
    int capacity;
} Stack;

/*
@brief inicializuje zásobník
*/
static void stack_init(Stack *stack) {
    stack->size = 0;
    stack->capacity = 8;
    stack->items = malloc(stack->capacity * sizeof(StackItem));
    if(stack->items == NULL) {
        fprintf(stderr, "memory allocation failed.\n");
        exit(99);
    }
}

/*
@brief uvolňuje zásobník
*/
static void stack_free(Stack *stack) {
    free(stack->items);
    stack->items = NULL;
    stack->size = 0;
    stack->capacity = 0;
}

/*
@brief pushne prvek na zásobník
@param stack zásobník, kam pushuje
@param item prvek k pushnutí
*/
static void stack_push(Stack *stack, StackItem item) {
    if(stack->size == stack->capacity) {
        stack->capacity *= 2;
        StackItem *tmp = realloc(stack->items, stack->capacity * sizeof(StackItem));
        if(tmp == NULL) {
            fprintf(stderr, "memory allocation failed.\n");
            exit(99);
        }
        stack->items = tmp;
    }
    stack->items[stack->size++] = item;
}

/*
@brief vrací index nejvyššího terminálu na zásobníku
@param stack zásobník, kde hledá nejvyšší terminál
@return i index nejvyššího terminálu
*/
static int stack_top_terminal(const Stack *stack) {
    for(int i = stack->size - 1; i>= 0; i--) {
        if(stack->items[i].kind == ITEM_TERMINAL) {
            return i;
        }
    }
    return -1;
}

/*
@brief vloží značku handle nad uvedený index
@param stack zásobník, kam vkládá handle
@param index_before index před handle (handle bude ZA tímto indexem)
*/
static void stack_insert_handle_above(Stack *stack, int index_before) {
    StackItem handle = { .kind = ITEM_HANDLE};
    stack_push(stack, handle);
    for (int i = stack->size - 1; i>index_before + 1; i--) {
        stack->items[i] = stack->items[i-1];
    }
    stack->items[index_before + 1] = handle;
}

/*
@brief převádí token na symbol z precedenční tabulky
@param type tup k převedení
@param prev_is_operand jestli byl předcházející token operand (aby se rozpoznalo unární mínus)
@param paren_depth počet nespojených závorek
*/
static PrecSymbol token_to_symbol(TokenType type, bool prev_is_operand, int paren_depth) {
    switch(type) {
        case TOKEN_LITERAL_INT:
        case TOKEN_LITERAL_DOUBLE:
        case TOKEN_LITERAL_STRING:
        case TOKEN_LITERAL_UNIT:
        case TOKEN_IDENTIFIER:
            return SYM_TERM;
        case TOKEN_MINUS:
            if(prev_is_operand) {
                return SYM_ADD_SUB;
            }
            return SYM_UNARY_MINUS;
        case TOKEN_PLUS:
            return SYM_ADD_SUB;
        case TOKEN_MUL:
        case TOKEN_DIV:
            return SYM_MUL_DIV;
        case TOKEN_LT:
        case TOKEN_LTE:
        case TOKEN_GT:
        case TOKEN_GTE:
            return SYM_REL;
        case TOKEN_EQ:
        case TOKEN_NEQ:
            return SYM_EQ;
        case TOKEN_LPAREN:
            return SYM_LPAREN;
        case TOKEN_RPAREN:
            if(paren_depth>0) {
                return SYM_RPAREN;
            }
            return SYM_END;
        default:
            return SYM_END;
    }
}

#define PREC_SIZE 9
static const char prec_table[PREC_SIZE][PREC_SIZE] = {
/*  	    	u-    +, -		*, /	rel	    !=,==	(	    )	    id	    $ */
/* u-       */ { '<',   '>',    '>',    '>',    '>',    '<',    '>',    '<',    '>' },
/* +,-      */ { '<',   '>',    '<',    '>',    '>',    '<',    '>',    '<',    '>' },
/* *, /     */ { '<',   '>',    '>',    '>',    '>',    '<',    '>',    '<',    '>' },
/* rel      */ { '<',   '<',    '<',    ' ',    '>',    '<',    '>',    '<',    '>' },
/* !=, ==   */ { '<',   '<',    '<',    '<',    ' ',    '<',    '>',    '<',    '>' },
/* (        */ { '<',   '<',    '<',    '<',    '<',    '<',    '=',    '<',    ' ' },
/* )        */ { ' ',   '>',    '>',    '>',    '>',    ' ',    '>',    ' ',    '>' },
/* id       */ { ' ',   '>',    '>',    '>',    '>',    ' ',    '>',    ' ',    '>' },
/* $        */ { '<',   '<',    '<',    '<',    '<',    '<',    ' ',    '<',    ' ' }
};

/*
@brief udělá uzel ast z tokenu
@return uzel se všemi odpovídajícími náležitostmi vyplněnými
*/
static ASTNode *make_term_node(Token token) {
    ASTNode *node;
    switch(token.Ttype) {
        case TOKEN_LITERAL_INT:
            node = ast_create_node(NODE_INT_LITERAL);
            node->int_value = token.Tatribut.in;
            break;
        case TOKEN_LITERAL_DOUBLE:
            node = ast_create_node(NODE_DOUBLE_LITERAL);
            node->double_value = token.Tatribut.db;
            break;
        case TOKEN_LITERAL_STRING:
            node = ast_create_node(NODE_STRING_LITERAL);
            node->string_value = token.Tatribut.string;
            break;
        case TOKEN_LITERAL_UNIT:
            node = ast_create_node(NODE_UNIT_LITERAL);
            break;
        case TOKEN_IDENTIFIER:
            node = ast_create_node(NODE_IDENTIFIER);
            node->name = token.Tatribut.string;
            break;
        default:
            syntax_error("invalid term in expression");
            return NULL;
    }
    return node;
}

/*
@brief kontroluje, jestli je symbol binární operátor
@param sym symbol ke kontrole
@return jestli je nebo není bin sym
*/
static bool is_binary_sym(PrecSymbol sym) {
    return sym == SYM_ADD_SUB || sym == SYM_MUL_DIV || sym == SYM_REL || sym == SYM_EQ;
}

/*
@brief udělá jednu redukci, najde nejvyšší handle a vyrobíl neterminál E
kostra pro tuto funkci byla vygenerována Claude Sonnet 5.5 
prompt: "Udělej mi prosím kostru funkce redukce pro precedenční syntaktickou analýzu"
*/
static void reduce(Stack *stack) {
    /*hledání handle*/
    int handle_index = -1;
    for(int i = stack->size -1; i >= 0; i--){
        if(stack->items[i].kind == ITEM_HANDLE) {
            handle_index = i;
            break;
        }
    }
    if (handle_index < 0) {
        syntax_error("invalid expression, handle not found.");
    }
    /*kolik prvků je nad handle*/
    int count = stack->size - handle_index - 1;
    /*část zásobníku, kde budě redukováno*/
    StackItem *to_reduce = &stack->items[handle_index + 1];
    ASTNode *result = NULL;
    
    if (count == 1 && to_reduce[0].kind == ITEM_TERMINAL && to_reduce[0].sym == SYM_TERM) {
        if (to_reduce[0].node != NULL) {
            result = to_reduce[0].node;
        } else {
            result = make_term_node(to_reduce[0].token);
        }
    }
    /*
        pro unární minus
        první symbol k redukci (index 0) je terminál (unírní minus)
        druhý prvek k redukci (index 1) je neterminál
    */
    else if (count == 2 && to_reduce[0].kind == ITEM_TERMINAL && to_reduce[0].sym == SYM_UNARY_MINUS 
        && to_reduce[1].kind == ITEM_NONTERMINAL){
        result = ast_create_node(NODE_UNARY_OP);
        result->op = to_reduce[0].token.Ttype;
        result->right = to_reduce[1].node;
    }
    else if (count == 3) {
        /* pokud jsou tyto tři symboly terminál (, neterminál, terminál ), musí se jednat o (E)*/
        if(to_reduce[0].kind == ITEM_TERMINAL && to_reduce[0].sym == SYM_LPAREN && 
            to_reduce[1].kind == ITEM_NONTERMINAL &&
            to_reduce[2].kind == ITEM_TERMINAL && to_reduce[2].sym == SYM_RPAREN) {
            result = to_reduce[1].node;
        }
        /*pokud jsou tyto tři symboly neterminál, terminál a zároveň binární symbol, neterminál, musí se jednat o binární operaci*/
        else if (to_reduce[0].kind == ITEM_NONTERMINAL && 
            to_reduce[1].kind == ITEM_TERMINAL && is_binary_sym(to_reduce[1].sym) && 
            to_reduce[2].kind == ITEM_NONTERMINAL) {
            result = ast_create_node(NODE_BINOP);
            result->op = to_reduce[1].token.Ttype;
            result->left = to_reduce[0].node;
            result->right = to_reduce[2].node;
        }
    }
    if(result == NULL) {
        syntax_error("invalid espression");
    }
    /* odstraní handle a zredukované prvky*/
    stack->size = handle_index;
    /*push reduced nonterminal E with the new ast node*/
    StackItem result_item = {
        .kind = ITEM_NONTERMINAL,
        .node = result
    };
    stack_push(stack, result_item);
}

/*
@brief parsuje výraz na základě precedence dané < > nebo =
*/
ASTNode *parse_expression(void){
    Stack stack;
    stack_init(&stack);
    StackItem bottom_item = {.kind = ITEM_TERMINAL, .sym = SYM_END};
    stack_push(&stack, bottom_item);

    bool prev_is_operand = false;
    int paren_depth = 0;

    while(true) {
        int top_term_index = stack_top_terminal(&stack);
        PrecSymbol stack_sym = stack.items[top_term_index].sym;
        PrecSymbol input_sym = token_to_symbol(current_token.Ttype, prev_is_operand, paren_depth);

        char relation = prec_table[stack_sym][input_sym];

        if(relation == '<' || relation == '=') {
            if (relation == '<') {
                stack_insert_handle_above(&stack, top_term_index);
            }
            StackItem item = { .kind = ITEM_TERMINAL, .sym = input_sym, .token = current_token};
            
            if(input_sym == SYM_TERM && current_token.Ttype == TOKEN_IDENTIFIER && peek_type() == TOKEN_LPAREN) {
                item.node = parse_func_call();
                stack_push(&stack, item);
                prev_is_operand = true;
                continue;
            }

            stack_push(&stack, item);

            if(input_sym == SYM_LPAREN) {
                paren_depth++;
            }
            if(input_sym == SYM_RPAREN) {
                paren_depth--;
            }
            prev_is_operand = (input_sym == SYM_TERM || input_sym == SYM_RPAREN);

            advance();
        }
        else if(relation == '>') {
            reduce(&stack);
        }
        else if(stack.size == 2 && stack.items[0].sym == SYM_END && stack.items[1].kind == ITEM_NONTERMINAL && input_sym == SYM_END) {
            ASTNode *result = stack.items[1].node;
            stack_free(&stack);
            return result;
        }
        else {
            stack_free(&stack);
            syntax_error("invalid or empty expression");
            return NULL;
        }
    }
}

