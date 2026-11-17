/*
Projekt: Implementace prekladace
author: Marek Janiga (xjanigm00)
Semanticka kontrola vyrazov
*/

#include "semantic.h"
#include <stddef.h> //null
static bool is_number(DataType t)
{
    return t == TYPE_INT || t == TYPE_DOUBLE;
}

static void set_to_double(ASTNode *expression)
{
    DataType l = expression->left->data_type;
    DataType r = expression->right->data_type;

    if (!(l == TYPE_INT && r == TYPE_INT)) {
        if (l == TYPE_INT) expression->left->to_double = true;
        if (r == TYPE_INT) expression->right->to_double = true;
    }
}

int sem_expression(ASTNode *expression, symtable_stack_t *scopes){
    if (expression == NULL) return INTERNAL_INTERPRET_ERR;

    switch(expression-> type){

        case NODE_INT_LITERAL:
            expression->data_type=TYPE_INT;
                return ERROR_OK;

        case NODE_DOUBLE_LITERAL: 
            expression->data_type=TYPE_DOUBLE;
                return ERROR_OK;

        case NODE_STRING_LITERAL:   
            expression->data_type=TYPE_STRING; 
                return ERROR_OK;

        case NODE_UNIT_LITERAL:   
            expression->data_type=TYPE_UNIT;
                return ERROR_OK;
        
        
        case NODE_IDENTIFIER:{
            symtable_data_t *found = NULL;
            int error = sem_lookup_var(scopes, expression, &found);
            if(error != ERROR_OK){
                return error;
            }
            expression->data_type=found->data_type;
            return ERROR_OK;
            }
        

        case NODE_UNARY_OP: {
            int error = sem_expression(expression->left, scopes);
            if (error != ERROR_OK) {
                return error;
            }

            
            if (expression->left->data_type != TYPE_DOUBLE && expression->left->data_type != TYPE_INT) return SEM_DIFF_TYPE_ERR;

            expression->data_type = expression->left->data_type;

            return ERROR_OK;
        }

        case NODE_BINOP: {
            int error = sem_expression(expression->left, scopes);
            if (error != ERROR_OK) {
                return error;
            }
            error = sem_expression(expression->right, scopes);
            if (error != ERROR_OK) {
                return error;
            }
            DataType data_l = expression->left->data_type;
            DataType data_r = expression->right->data_type;

           
            switch (expression->op) {
                // aritmetika
                case TOKEN_PLUS:
                case TOKEN_MINUS:
                case TOKEN_MUL:
                case TOKEN_DIV:
                    if (is_number(data_l) && is_number(data_r)) {
                        set_to_double(expression);
                        if (data_l == TYPE_INT && data_r == TYPE_INT) {
                            expression->data_type = TYPE_INT;
                        } else {
                            expression->data_type = TYPE_DOUBLE;
                        }
                    } else if (data_l == TYPE_STRING && data_r == TYPE_STRING
                            && expression->op == TOKEN_PLUS) {
                        expression->data_type = TYPE_STRING;
                    } else {
                        return SEM_DIFF_TYPE_ERR;
                    }
                    break;
 
                // relacne operatory
                case TOKEN_LT:
                case TOKEN_LTE:
                case TOKEN_GT:
                case TOKEN_GTE:
                    if (is_number(data_l) && is_number(data_r)) {
                        set_to_double(expression);
                        expression->data_type = TYPE_BOOL;
                    } else if (data_l == TYPE_STRING && data_r == TYPE_STRING) {
                        expression->data_type = TYPE_BOOL;
                    } else {
                        return SEM_DIFF_TYPE_ERR;
                    }
                    break;

                // rovnost
                case TOKEN_EQ:
                case TOKEN_NEQ:
                    if (is_number(data_l) && is_number(data_r)) {
                        set_to_double(expression);
                        expression->data_type = TYPE_BOOL;
                    } else if (data_l == data_r && data_l != TYPE_BOOL) {
                        expression->data_type = TYPE_BOOL;
                    } else {
                        return SEM_DIFF_TYPE_ERR;
                    }
                    break;

                default:
                    return INTERNAL_INTERPRET_ERR;
            }
            return ERROR_OK;
        }

        default:
                return INTERNAL_INTERPRET_ERR;
       
    }
}