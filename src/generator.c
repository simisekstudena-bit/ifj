/*
Projekt: Implementace prekladace
@file generator.c
@author Patrik Kadleček (xkadlep01)
@brief generacia finalneho kodu
*/

#include "generator.h"
#include "errors.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/*
===== LINKED LIST =====
inspiracia z druhej prednasky IAL
*/

// uzol
typedef struct node {
    char *instruction;
    struct node *next;
} node_t;

// zoznam
typedef struct list {
    node_t *first;
    node_t *last; // ukladame posledny prvok na rychle appendovanie
} list_t;

// zoznam vsetkych vygenerovanych instrukcii
static list_t code;

/*
@brief inicializovanie zoznamu
@param l pointer na zoznam
*/
static void list_init(list_t *l) {
    l->first = NULL;
    l->last = NULL;
}

/*
@brief pridavanie prvkov do zoznamu
@param l zoznam
@param text prvok
*/
static void list_append(list_t *l, char *text) {
    node_t *new_node = malloc(sizeof(node_t));

    if (new_node == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        free(text);
        exit(INTERNAL_INTERPRET_ERR);
    }
    // zoznam prevezme vlastnictvo instrukcie
    new_node->instruction = text;
    new_node->next = NULL;

    // ziadne prvky
    if (l->first == NULL) {
        l->first = new_node;
        l->last = new_node;
    } else { // jeden alebo viac prvkov
        l->last->next = new_node;
        l->last = new_node;
    }
}

/*
@brief uvolnenie zoznamu
@param l zoznam ktory chceme uvolnit
*/
static void list_free(list_t *l) {
    while (l->first != NULL) {
        node_t *current = l->first;
        l->first = l->first->next;
        free(current->instruction);
        free(current);
    }
    l->last = NULL;
}

/*
@brief formatuje retazec podla va_list a alokuje buffer
@param format formatovaci retazec ("PUSHS int@%d")
@param args va_list s hodnotami pre format
@return novo alokovany retazec (uvolni ho volajuci)
*/
static char *vformat(const char *format, va_list args) {
    va_list args2;
    va_copy(args2, args);

    // zmeria dlzku vysledneho sformatovaneho retazca
    int len = vsnprintf(NULL, 0, format, args);
    if (len < 0) {
        fprintf(stderr, "Failed to load instruction length\n");
        exit(INTERNAL_INTERPRET_ERR);
    }

    // vytvorenie buffera pre instrukciu
    char *buf = malloc(len + 1);
    if (buf == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(INTERNAL_INTERPRET_ERR);
    }

    // zapise formatovanu instrukciu do bufferu
    vsnprintf(buf, len + 1, format, args2);
    va_end(args2);
    return buf;
}

/*
@brief sformatuje retazec ako printf a vrati ho alokovany (obal nad vformat pre ...)
@param format formatovaci retazec ("PUSHS int@%d")
@return vrati alokovany retazec
*/
static char *alloc_format(const char *format, ...) {
    va_list args;
    va_start(args, format);

    char *result = vformat(format, args);
    va_end(args);
    return result;
}

/*
@brief funkcia pridavanie instrukcii do zoznamu
@param format format instrukcie
*/
static void add_instruction(const char *format, ...) {
    va_list args;
    va_start(args, format);

    list_append(&code, vformat(format, args));
    va_end(args);
}

/*
@brief formatuje znaky 0-32, # a \ na \xyz format
@param string povodny retazec z AST
@return novy alokovany retazec v \xyz formate
*/
static char *format_string_literal(const char *string) {
    size_t len = strlen(string);
    char *formatted = malloc(4*len+1);
    if(formatted == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(INTERNAL_INTERPRET_ERR);
    }
    int pos = 0;

    for(size_t i = 0; i < len; i++) {
        unsigned char ch = string[i];
        if(ch <= 32 || ch == '\\' || ch == '#') {
            sprintf(formatted + pos, "\\%03d", ch);
            pos += 4;
        } else {
            formatted[pos++] = ch;
        }
    }
    formatted[pos] = '\0';
    return formatted;
}

/*
@brief z literalu alebo premennej spravi operand (int@5)
@param node uzol literalu (Int, Double, String, Unit) alebo premennej
@return alokovany operand (napr. int@5)
*/
static char *format_literal(ASTNode *node) {
    switch (node->type) {
        case NODE_INT_LITERAL:
            return alloc_format("int@%d", node->int_value);
        case NODE_DOUBLE_LITERAL:
            return alloc_format("float@%a", node->double_value);
        case NODE_STRING_LITERAL: {
            char *formatted = format_string_literal(node->string_value);
            char *operand = alloc_format("string@%s", formatted);
            free(formatted);
            return operand;
        }
        case NODE_UNIT_LITERAL:
            return alloc_format("nil@nil");
        case NODE_IDENTIFIER:
            return alloc_format("LF@%s", node->unique_name);

        default:
            fprintf(stderr, "format_literal: not a literal node\n");
            exit(INTERNAL_INTERPRET_ERR);
    }
}

/*
    ====== GENERACIA KODU ======
*/

// pocitadlo pre if/while
static int label_counter = 0;

/*
@brief pocitadlo pre if/while
@return cislo pre if/while
*/
static int new_label(void) {
    label_counter += 1;
    return label_counter;
}

/*
@brief inicializacia zoznamu
*/
void gen_init(void) {
    list_init(&code);
    add_instruction(".IFJcode26");
}

/*
@brief vypise zoznam instrukcii
@param out vystupny subor
*/
void gen_print(FILE *out) {
    node_t *current = code.first;

    while (current != NULL) {
        fprintf(out, "%s\n", current->instruction);
        current = current->next;
    }
}

/*
@brief uvolni zoznam
*/
void gen_free(void) {
    list_free(&code);
}

/*
@brief prejde blok a pre kazdu premennu vygeneruje DEFVAR
@param node blok (telo funkcie alebo vetva if/while)
*/
static void defvar_load(ASTNode *node) {
    if (node == NULL)
        return;
    ASTNode *current = node->body;
    while(current != NULL) {
        switch (current->type) {
            case NODE_VAR_DECL:
                add_instruction("DEFVAR LF@%s", current->unique_name);
                break;
            case NODE_IF:
                defvar_load(current->body);
                defvar_load(current->else_body);
                break;
            case NODE_WHILE:
                defvar_load(current->body);
                break;
            default:
                break;
        }
        current = current->next;
    }
}

/*
@brief vygeneruje koniec funkcie
@param node uzol funkcie (podla is_main sa rozhodne medzi EXIT a RETURN)
*/
static void gen_function_end(ASTNode *node) {
    if(node->is_main) {
        add_instruction("EXIT int@0");
    } else {
        add_instruction("PUSHS nil@nil");
        add_instruction("POPFRAME");
        add_instruction("RETURN");
    }
}

/*
@brief vygeneruje vysledok na datovy zasobnik
@param node uzol vyrazu
*/
static void gen_expression(ASTNode *node) {
    switch (node->type) {
    case NODE_INT_LITERAL: {
        char * expression = format_literal(node);
        add_instruction("PUSHS %s", expression);
        free(expression);
        break;
    }
    case NODE_DOUBLE_LITERAL: {
        char * expression = format_literal(node);
        add_instruction("PUSHS %s", expression);
        free(expression);
        break;
    }
    case NODE_STRING_LITERAL: {
        char * expression = format_literal(node);
        add_instruction("PUSHS %s", expression);
        free(expression);
        break;
    }
    case NODE_UNIT_LITERAL: {
        char * expression = format_literal(node);
        add_instruction("PUSHS %s", expression);
        free(expression);
        break;
    }
    case NODE_IDENTIFIER: {
        char * expression = format_literal(node);
        add_instruction("PUSHS %s", expression);
        free(expression);
        break;
    }

    case NODE_UNARY_OP:
        if(node->data_type == TYPE_DOUBLE)
            add_instruction("PUSHS float@0x0p+0");
        else
            add_instruction("PUSHS int@0");
        gen_expression(node->right);
        add_instruction("SUBS");
        break;

    case NODE_BINOP: {
        gen_expression(node->left);
        gen_expression(node->right);
        switch (node->op) {
        case TOKEN_PLUS:
            if(node->data_type == TYPE_STRING)
                add_instruction("CALL $concat");
            else
                add_instruction("ADDS");
            break;
        case TOKEN_MINUS:
            add_instruction("SUBS");
            break;
        case TOKEN_MUL:
            add_instruction("MULS");
            break;
        case TOKEN_DIV:
            if(node->data_type == TYPE_DOUBLE)
                add_instruction("DIVS");
            else
                add_instruction("CALL $idiv");
            break;
        case TOKEN_LT:
            add_instruction("LTS");
            break;
        case TOKEN_GT:
            add_instruction("GTS");
            break;
        case TOKEN_EQ:
            add_instruction("EQS");
            break;
        case TOKEN_NEQ:
            add_instruction("EQS");
            add_instruction("NOTS");
            break;
        case TOKEN_LTE:
            add_instruction("GTS");
            add_instruction("NOTS");
            break;
        case TOKEN_GTE:
            add_instruction("LTS");
            add_instruction("NOTS");
            break;
        default:
            fprintf(stderr, "gen_expression: unsupported operator\n");
            exit(INTERNAL_INTERPRET_ERR);
        }
        break;
    }
    case NODE_FUNC_CALL: {
        ASTNode *arg = node->params;
        while(arg != NULL) {
            gen_expression(arg);
            arg = arg->next;
        }
        add_instruction("CALL %s",node->name);
        break;
    }

    default:
        fprintf(stderr, "gen_expression: unsupported node type\n");
        exit(INTERNAL_INTERPRET_ERR);
    }
    if(node->to_double)
        add_instruction("INT2FLOATS");
}

/*
@brief vyberie parametre zo zasobnika od posledneho
@param param prvy parameter funkcie
*/
static void pop_params(ASTNode *param) {
    if(param == NULL) return;
    pop_params(param->next);
    add_instruction("POPS LF@%s", param->unique_name);
}

static void gen_statement(ASTNode *node);

/*
@brief vygeneruje blok kodu (napr. if/while)
@param block blok (NODE_BLOCK), prikazy v body
*/
static void gen_block(ASTNode *block) {
    if(block == NULL) return;
    ASTNode *statement = block->body;
    while(statement != NULL) {
        gen_statement(statement);
        statement = statement->next;
    }
}

// funkcia na ktorej sa prave nachadzame
static ASTNode *current_function = NULL;

/*
@brief vygeneruje jeden prikaz
@param node jeden prikaz (vyraz, deklaracia, if, while, return)
*/
static void gen_statement(ASTNode *node) {
    if (node == NULL) {
        return;
    }
    switch (node->type) {
        case NODE_EXPR_STMT: {
            // Napisane podla pseudokodu od Claude Opus 5.5 s promptom:
            // "Vygeneruj pseudokod pre case NODE_EXPR_STMT"
            ASTNode *expr = node->left;
            if (expr->type == NODE_FUNC_CALL && strcmp(expr->name, "print") == 0 ) {
                ASTNode *arg = expr->params;
                gen_expression(arg);
                add_instruction("POPS GF@$trash");
                if(arg->type == NODE_UNIT_LITERAL || arg->data_type == TYPE_UNIT) {
                    add_instruction("WRITE string@()");
                }
                else
                    add_instruction("WRITE GF@$trash");
            }
            else {
                gen_expression(expr);
                add_instruction("POPS GF@$trash");
            }
            break;
        }
        case NODE_VAR_DECL:
            if(node->left != NULL) {
                gen_expression(node->left);
                add_instruction("POPS LF@%s", node->unique_name);
            }
            break;
        case NODE_ASSIGN:
            if(node->left != NULL) {
                gen_expression(node->left);
                add_instruction("POPS LF@%s", node->unique_name);
            }
            break;
        case NODE_IF: {
            int n = new_label();
            gen_expression(node->cond);
            add_instruction("PUSHS bool@false");
            add_instruction("JUMPIFEQS if$%d$else", n);
            gen_block(node->body);
            add_instruction("JUMP if$%d$end", n);
            add_instruction("LABEL if$%d$else", n);
            gen_block(node->else_body);
            add_instruction("LABEL if$%d$end", n);
            break;
        }
        case NODE_WHILE: {
            int n = new_label();
            add_instruction("LABEL while$%d$start", n);
            gen_expression(node->cond);
            add_instruction("PUSHS bool@false");
            add_instruction("JUMPIFEQS while$%d$end", n);
            gen_block(node->body);
            add_instruction("JUMP while$%d$start", n);
            add_instruction("LABEL while$%d$end", n);
            break;
        }
        // Vygenerovane pomocou Claude Opus 5.5 s promptom:
        // "vygeneruj pseudokod na opraveny kod pre case NODE_RETURN"
        case NODE_RETURN:
            if(node->left != NULL)
                gen_expression(node->left);

            if(current_function->is_main) {
                if(node->left != NULL)
                    add_instruction("POPS GF@$trash");
                add_instruction("EXIT int@0");
                break;
            }

            if(current_function->data_type == TYPE_UNIT) {
                if(node->left != NULL)
                    add_instruction("POPS GF@$trash");
                add_instruction("PUSHS nil@nil");
            }

            add_instruction("POPFRAME");
            add_instruction("RETURN");
            break;

        default:
            break;
    }
}


/*
@brief vygeneruje kod pre funkciu
@param node uzol funkcie: meno, parametre, telo, typ, is_main
*/
static void gen_function(ASTNode *node) {
    current_function = node;
    add_instruction("LABEL %s", node->name);
    add_instruction("CREATEFRAME");
    add_instruction("PUSHFRAME");

    // Definuje parametre
    ASTNode *param = node->params;
    while(param != NULL) {
        add_instruction("DEFVAR LF@%s", param->unique_name);
        param = param->next;
    }
    param = node->params;
    pop_params(param);

    // Vygenerovane pomocou Claude Opus 5.5 s promptom: "Vygeneruj koniec funkcie,
    // ktory vrati hodnotu posledneho vyrazu v tele funkcie."
    // Vygeneruje telo; funkcia, ktora nie je main ani Unit, vrati hodnotu
    // posledneho prikazu, ak je tento prikaz vyraz
    if (node->body != NULL && node->body->type == NODE_BLOCK) {
        defvar_load(node->body);
        ASTNode *statement = node->body->body;
        while(statement != NULL) {
            if(statement->next == NULL && !node->is_main && node->data_type != TYPE_UNIT
               && statement->type == NODE_EXPR_STMT) {
                gen_expression(statement->left);
                add_instruction("POPFRAME");
                add_instruction("RETURN");
                return;
            }
            gen_statement(statement);
            statement = statement->next;
        }
    }

    gen_function_end(node);
}

/*
@brief pomocna funkcia pre generovanie zaciatku vstavanych funkcii
*/
static void gen_builtins_start(void) {
    add_instruction("CREATEFRAME");
    add_instruction("PUSHFRAME");
    add_instruction("DEFVAR LF@r");
}

/*
@brief pomocna funkcia pre generovanie konca vstavanych funkcii
*/
static void gen_builtins_end(void) {
    add_instruction("PUSHS LF@r");
    add_instruction("POPFRAME");
    add_instruction("RETURN");
}

/*
@brief readInt(): Int
*/
static void gen_builtins_readInt(void) {
    add_instruction("LABEL readInt");
    gen_builtins_start();
    add_instruction("READ LF@r int");
    add_instruction("JUMPIFNEQ readInt$ok LF@r nil@nil");
    add_instruction("MOVE LF@r int@0");
    add_instruction("LABEL readInt$ok");
    gen_builtins_end();
}

/*
@brief readDouble(): Double
*/
static void gen_builtins_readDouble(void) {
    add_instruction("LABEL readDouble");
    gen_builtins_start();
    add_instruction("READ LF@r float");
    add_instruction("JUMPIFNEQ readDouble$ok LF@r nil@nil");
    add_instruction("MOVE LF@r float@0x0p+0");
    add_instruction("LABEL readDouble$ok");
    gen_builtins_end();
}

/*
@brief readLine(): String
*/
static void gen_builtins_readLine(void) {
    add_instruction("LABEL readLine");
    gen_builtins_start();
    add_instruction("READ LF@r string");
    gen_builtins_end();
}

/*
@brief toStr(value: Any): String
*/
static void gen_builtins_toStr(void) {
    add_instruction("LABEL toStr");
    gen_builtins_start();
    add_instruction("DEFVAR LF@v");
    add_instruction("DEFVAR LF@t");
    add_instruction("POPS LF@v");
    // Urcenie typu
    add_instruction("TYPE LF@t LF@v");
    add_instruction("JUMPIFEQ toStr$int LF@t string@int");
    add_instruction("JUMPIFEQ toStr$float LF@t string@float");
    add_instruction("JUMPIFEQ toStr$string LF@t string@string");
    // ak ziadny skok neprebehol (nil)
    add_instruction("MOVE LF@r string@()");
    add_instruction("JUMP toStr$end");
    // Int
    add_instruction("LABEL toStr$int");
    add_instruction("INT2STR LF@r LF@v");
    add_instruction("JUMP toStr$end");
    // Double
    add_instruction("LABEL toStr$float");
    add_instruction("FLOAT2STR LF@r LF@v");
    add_instruction("JUMP toStr$end");
    // String
    add_instruction("LABEL toStr$string");
    add_instruction("MOVE LF@r LF@v");

    add_instruction("LABEL toStr$end");
    gen_builtins_end();
}

/*
@brief toInt(value: Double): Int
*/
static void gen_builtins_toInt(void) {
    add_instruction("LABEL toInt");
    gen_builtins_start();
    add_instruction("DEFVAR LF@v");
    add_instruction("POPS LF@v");
    add_instruction("FLOAT2INT LF@r LF@v");
    gen_builtins_end();
}

/*
@brief strLength(value: String): Int
*/
static void gen_builtins_strLenght(void) {
    add_instruction("LABEL strLength");
    gen_builtins_start();
    add_instruction("DEFVAR LF@s");
    add_instruction("POPS LF@s");
    add_instruction("STRLEN LF@r LF@s");
    gen_builtins_end();
}

// Napisane podla pseudokodu od Claude Opus 5.5 s promptom:
// "vygeneruj mi pseudokod pre funkciu gen_builtins_subStr"
/*
@brief subStr(value: String, start: Int, end: Int): String
*/
static void gen_builtins_subStr(void) {
    add_instruction("LABEL subStr");
    gen_builtins_start();
    // parametre
    add_instruction("DEFVAR LF@s");
    add_instruction("DEFVAR LF@start");
    add_instruction("DEFVAR LF@end");
    // pomocne premenne
    add_instruction("DEFVAR LF@len");
    add_instruction("DEFVAR LF@tmp");
    add_instruction("DEFVAR LF@idx");
    add_instruction("DEFVAR LF@ch");
    add_instruction("POPS LF@end");
    add_instruction("POPS LF@start");
    add_instruction("POPS LF@s");
    add_instruction("STRLEN LF@len LF@s");
    // kontroly
    // start < 0
    add_instruction("LT LF@tmp LF@start int@0");
    add_instruction("JUMPIFEQ subStr$err LF@tmp bool@true");
    // end < 0
    add_instruction("LT LF@tmp LF@end int@0");
    add_instruction("JUMPIFEQ subStr$err LF@tmp bool@true");
    // start > end
    add_instruction("GT LF@tmp LF@start LF@end");
    add_instruction("JUMPIFEQ subStr$err LF@tmp bool@true");
    // start >= strLength(value), testovane ako !(start < len)
    add_instruction("LT LF@tmp LF@start LF@len");
    add_instruction("JUMPIFEQ subStr$err LF@tmp bool@false");
    // end > strLength(value)
    add_instruction("GT LF@tmp LF@end LF@len");
    add_instruction("JUMPIFEQ subStr$err LF@tmp bool@true");

    add_instruction("MOVE LF@r string@");
    add_instruction("MOVE LF@idx LF@start");
    add_instruction("LABEL subStr$loop");
    add_instruction("LT LF@tmp LF@idx LF@end");
    add_instruction("JUMPIFEQ subStr$end LF@tmp bool@false");
    add_instruction("GETCHAR LF@ch LF@s LF@idx");
    add_instruction("CONCAT LF@r LF@r LF@ch");
    add_instruction("ADD LF@idx LF@idx int@1");
    add_instruction("JUMP subStr$loop");

    add_instruction("LABEL subStr$err");
    add_instruction("EXIT int@40");
    add_instruction("LABEL subStr$end");

    gen_builtins_end();
}

/*
@brief ord(value: String, index: Int): Int
*/
static void gen_builtins_ord(void) {
    add_instruction("LABEL ord");
    gen_builtins_start();
    add_instruction("DEFVAR LF@s");
    add_instruction("DEFVAR LF@i");
    // pomocne premenne
    add_instruction("DEFVAR LF@len");
    add_instruction("DEFVAR LF@tmp");
    add_instruction("POPS LF@i");
    add_instruction("POPS LF@s");
    add_instruction("STRLEN LF@len LF@s");
    // ak je mensie ako 0
    add_instruction("LT LF@tmp LF@i int@0");
    add_instruction("JUMPIFEQ ord$bad LF@tmp bool@true");
    // index >= dlzka
    add_instruction("LT LF@tmp LF@i LF@len");
    add_instruction("JUMPIFEQ ord$bad LF@tmp bool@false");
    add_instruction("STRI2INT LF@r LF@s LF@i");
    add_instruction("JUMP ord$end");
    add_instruction("LABEL ord$bad");
    add_instruction("MOVE LF@r int@-1");
    add_instruction("LABEL ord$end");
    gen_builtins_end();
}

/*
@brief chr(code: Int): String
*/
static void gen_builtins_chr(void) {
    add_instruction("LABEL chr");
    gen_builtins_start();
    add_instruction("DEFVAR LF@code");
    add_instruction("POPS LF@code");
    add_instruction("INT2CHAR LF@r LF@code");
    gen_builtins_end();
}

/*
@brief interna funkcia $concat pre + medzi retazcami
*/
static void gen_builtins_concat(void) {
    add_instruction("LABEL $concat");
    gen_builtins_start();
    add_instruction("DEFVAR LF@a");
    add_instruction("DEFVAR LF@b");
    add_instruction("POPS LF@b");
    add_instruction("POPS LF@a");
    add_instruction("CONCAT LF@r LF@a LF@b");
    gen_builtins_end();
}

// Napisane podla pseudokodu od Claude Opus 5.5 s promptom:
// "vygeneruj pseudokod pre funkciu idiv ktora bude zaokruhlovat k nule".

/*
@brief interna funkcia na celociselne delenie so zaokruhlovanim k nule
*/
static void gen_builtins_idiv(void) {
    add_instruction("LABEL $idiv");
    gen_builtins_start();
    add_instruction("DEFVAR LF@a");
    add_instruction("DEFVAR LF@b");
    add_instruction("POPS LF@b");
    add_instruction("POPS LF@a");
    add_instruction("INT2FLOAT LF@a LF@a");
    add_instruction("INT2FLOAT LF@b LF@b");
    add_instruction("DIV LF@a LF@a LF@b");
    add_instruction("FLOAT2INT LF@r LF@a");
    gen_builtins_end();
}

/*
@brief funkcia ktora vygeneruje vstavane funkcie
*/
static void gen_builtins(void) {
    gen_builtins_readInt();
    gen_builtins_readLine();
    gen_builtins_readDouble();
    gen_builtins_toStr();
    gen_builtins_toInt();
    gen_builtins_strLenght();
    gen_builtins_subStr();
    gen_builtins_ord();
    gen_builtins_chr();
    gen_builtins_concat();
    gen_builtins_idiv();
}

/*
@brief funkcia na generaciu finalneho programu
@param root koren programu
*/
void gen_program(ASTNode *root) {

    ASTNode *node = root->body;
    while(node != NULL) {
        if(node->is_main) {
            add_instruction("DEFVAR GF@$trash");
            add_instruction("JUMP %s", node->name);
            break;
        }
        node = node->next;
    }
    gen_builtins();

    node = root->body;
    while(node != NULL) {
        gen_function(node);
        node = node->next;
    }
}
