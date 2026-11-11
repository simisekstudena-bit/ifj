#ifndef scanner_h
   #define scanner_h

#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    STATE_LINE_START,
    STATE_IDENTIFIER,
    STATE_NUMBER,
    STATE_NUMBER_UNDERSCORE,
    STATE_DOUBLE,
    STATE_EQUALS,
    STATE_LESS,
    STATE_GREATER,
    STATE_NOT,
    STATE_SLASH,
    STATE_LINE_COMMENT,
    STATE_BLOCK_COMMENT,
    STATE_BLOCK_COMMENT_STAR,
    STATE_BLOCK_COMMENT_SLASH,
    STATE_DOUBLE_DIGITS,
    STATE_EXP,
    STATE_EXP_SIGN,
    STATE_EXP_DIGITS,
    STATE_DOUBLE_UNDERSCORE,
    STATE_EXP_UNDERSCORE,
    STATE_START,
    STATE_STRING_START,
    STATE_STRING_QUOTES2,
    STATE_STRING,
    STATE_MULTILINE_STRING,
    STATE_MULTILINE_QUOTES,
    STATE_DOT,\
    STATE_PAREN_LINE_START,
    STATE_STRING_ESCAPE
} ScannerState;

typedef enum {
    TOKEN_EOF,              // Koniec súboru
    TOKEN_EOL,              // \n 
    TOKEN_INDENT,           // Začiatok bloku (väčšie odsadenie)
    TOKEN_DEDENT,           // Koniec bloku (menšie odsadenie)

    TOKEN_IDENTIFIER,       // Názov
    TOKEN_LITERAL_INT,      // 42
    TOKEN_LITERAL_DOUBLE,   // 3.14
    TOKEN_LITERAL_STRING,   // "text"
    TOKEN_LITERAL_UNIT,     // ()

    TOKEN_KW_DEF,           // def
    TOKEN_KW_DO,            // do
    TOKEN_KW_ELSE,          // else
    TOKEN_KW_IF,            // if
    TOKEN_KW_IMPORT,        // import
    TOKEN_KW_RETURN,        // return
    TOKEN_KW_THEN,          // then
    TOKEN_KW_VAL,           // val
    TOKEN_KW_VAR,           // var
    TOKEN_KW_WHILE,         // while

    TOKEN_KW_INT,           // Int
    TOKEN_KW_DOUBLE,        // Double
    TOKEN_KW_STRING,        // String
    TOKEN_KW_UNIT,          // Unit

    TOKEN_ASSIGN,           // =
    TOKEN_PLUS,             // +
    TOKEN_MINUS,            // -
    TOKEN_MUL,              // *
    TOKEN_DIV,              // /

    TOKEN_EQ,               // ==
    TOKEN_NEQ,              // !=
    TOKEN_LT,               // <
    TOKEN_LTE,              // <=
    TOKEN_GT,               // >
    TOKEN_GTE,              // >=

    TOKEN_LPAREN,           // (
    TOKEN_RPAREN,           // )
    TOKEN_COLON,            // :
    TOKEN_COMMA,            // ,
    TOKEN_SEMICOLON,        // ;
    TOKEN_DOT,              // .
    TOKEN_AT_MAIN,          // @main
    TOKEN_IFJ26             // ifj26

} TokenType;

typedef union{

int in;
double db;
char *string;

}TokenAtribut;



typedef struct{

TokenType Ttype;
TokenAtribut Tatribut;

}Token; 


typedef struct {
    const char *name;
    TokenType type;
} Keyword;




Token get_token(FILE *source_file);

void free_token(Token *t);

TokenType lookup_keyword(char *str);


  #endif


