#include "scanner.h"


Keyword keywords[] = {
    {"def", TOKEN_KW_DEF},
    {"do", TOKEN_KW_DO},
    {"else", TOKEN_KW_ELSE},
    {"if", TOKEN_KW_IF},
    {"import", TOKEN_KW_IMPORT},
    {"return", TOKEN_KW_RETURN},
    {"then", TOKEN_KW_THEN},
    {"val", TOKEN_KW_VAL},
    {"var", TOKEN_KW_VAR},
    {"while", TOKEN_KW_WHILE},
    {"Int", TOKEN_KW_INT},
    {"Double", TOKEN_KW_DOUBLE},
    {"String", TOKEN_KW_STRING},
    {"Unit", TOKEN_KW_UNIT},
    {"ifj26",TOKEN_IFJ26},
};

static int paren_depth = 0;
static int comment_depth = 0;
static ScannerState state = STATE_LINE_START;
static bool comment_has_newline = false;
static int space_count = 0;
static bool tab_in_indent = false;
static bool comment_from_paren = false;
//odsadenia 
static int indent_stack[100] = {0};
static int indent_top = 0;  

void buf_add(char *buffer, int *length, int c) {
    if (*length + 1 > 255) {
        fprintf(stderr, "Lexikalna chyba 1: cislo je prilis dlhe\n");
        exit(1);
    }
    buffer[(*length)++] = (char)c;
    buffer[*length] = '\0';
}


void free_token(Token *t) {
    if ((t->Ttype == TOKEN_LITERAL_STRING || t->Ttype == TOKEN_IDENTIFIER) && t->Tatribut.string != NULL) {
        free(t->Tatribut.string);
        t->Tatribut.string = NULL;
    }
}

TokenType lookup_keyword(char *str){
    long int n = sizeof(keywords) / sizeof(keywords[0]);
    for (long int i = 0; i < n; i++) {
        if (strcmp(str, keywords[i].name) == 0)
            return keywords[i].type;
    }
    return TOKEN_IDENTIFIER;

}


Token get_token(FILE *source_file){
    Token Ttoken;
    int c;
    char buffer[256];
    int length = 0;
    int quotes = 0;
    bool at_sign = false;

    int cap = 0;
    char *str = NULL;

    

    while (true){
    
    c=fgetc(source_file);

    switch (state){

        case STATE_LINE_START:
           
           
            if (c == EOF){
                if (indent_top > 0) {
                    indent_top--;
                    Ttoken.Ttype = TOKEN_DEDENT;
                    Ttoken.Tatribut.in = indent_stack[indent_top];
                    return Ttoken;
                }
            Ttoken.Ttype = TOKEN_EOF;
            return Ttoken;

                }

         
            else if (c == ' '){
                space_count ++;

            }

            
            else if (c == '\t'){
                tab_in_indent = true;
            }
            else if (c == '\n'|| c == '\r') {
                tab_in_indent = false;
                space_count = 0;
                continue;
            }
            else{
                if (c == '/') {
                int next = fgetc(source_file);
                //riadkovy komentar
                if (next == '/') {
                     
                    while ((c = fgetc(source_file)) != '\n' && c != EOF);
                    space_count = 0;
                    tab_in_indent = false;
                    continue;
                }
                else if (next == '*') {
                    //blokovy komentar aj s vnorenymi 
                    comment_has_newline = false;
                    int depth = 1;
                    int prev = 0;
                    while (depth > 0) {
                        c = fgetc(source_file);
                        if (c == '\n') comment_has_newline = true;
                        if (c == EOF) {
                            fprintf(stderr, "Lexikalna chyba 1: neukonceny blokovy komentar\n");
                            exit(1);
                        }

                        if (prev == '/' && c == '*'){
                         depth++; 
                         prev = 0; 
                        }

                        else if (prev == '*' && c == '/'){
                         depth--; 
                         prev = 0; 
                        }

                        else prev = c;
                    }

                    do {
                        c = fgetc(source_file);
                    } while (c == ' ' || c == '\t');

                    
                    if (comment_has_newline && c != '\n' && c != '\r' && c != EOF && c != '/') {
                        fprintf(stderr, "Lexikalna chyba 1: za blokovym komentarom s novym riadkom nesmie byt kod\n");
                        exit(1);
                    }
                    comment_has_newline = false;

                    // zvysok riadku spracuje STATE_LINE_START odznova
                    ungetc(c, source_file);
                    continue;
                    
                }
                else {
                    ungetc(next, source_file);
                }
            }
            ungetc(c, source_file);
            if (tab_in_indent) {
                fprintf(stderr, "Lexikalna chyba 1: Tabulator v odsadeni\n");
                exit(1);
            }
            
    

            //ident 
            if (space_count > indent_stack[indent_top]) {
                 if (indent_top + 1 >= 100) {
                    fprintf(stderr, "Interna chyba kompilatora: prilis vela urovni odsadenia\n");
                    exit(99);
                }
                indent_stack[++indent_top] = space_count;
                Ttoken.Ttype = TOKEN_INDENT;
                Ttoken.Tatribut.in = space_count;
                space_count = 0;
                state = STATE_START;
                return Ttoken;
            }

            //dedent
            if (space_count < indent_stack[indent_top]){
                indent_top --;
                if (space_count > indent_stack[indent_top]) {
                    fprintf(stderr, "Syntakticka chyba 2: odsadenie nezodpoveda ziadnej urovni\n");
                    exit(2);
                }
                Ttoken.Ttype = TOKEN_DEDENT;
                Ttoken.Tatribut.in = indent_stack[indent_top];
                
                return Ttoken;
            }
            space_count = 0;
            state = STATE_START;


            }
            
            break;


        case STATE_START:
            
            if (c == ' ' || c == '\t')break;
            
            if (paren_depth == 0 && comment_has_newline && c != '\n' && c != '\r' && c != EOF && c != '/')  {
                fprintf(stderr, "Lexikalna chyba 1: za blokovym komentarom s novym riadkom nesmie byt kod\n");
                exit(1);
            }
            else if (c == '\n' || c == '\r') {
                if (paren_depth > 0) {
                    comment_has_newline = false;
                    state = STATE_PAREN_LINE_START;
                    break;
                }
                                    
                comment_has_newline = false;
                Ttoken.Ttype = TOKEN_EOL;
                state = STATE_LINE_START;
                return Ttoken;
            }
            else if (c == EOF) {
                comment_has_newline = false;
                Ttoken.Ttype = TOKEN_EOL;
                state = STATE_LINE_START;
                return Ttoken;
            }


            else if (c == '@') {
                c = fgetc(source_file);
                if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
                    fprintf(stderr, "Lexikalna chyba 1: za @ musi nasledovat main\n");
                    exit(1);
                }
                at_sign = true;
                cap = 16;
                str = malloc(cap);
                if (str == NULL) {
                    fprintf(stderr, "Chyba kompilatora: zlyhala alokacia pamate\n");
                    exit(99);
                }
                str[0] = (char)c;
                str[1] = '\0';
                length = 1;
                state = STATE_IDENTIFIER;
                break;
            }
            
            else if (c == '.') {
                buffer[0] = '.';
                length = 1;
                buffer[length] = '\0';
                state = STATE_DOT;
                break;
            }

            else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
            
            cap = 16;
            str = malloc(cap);
            if (str == NULL) {
                fprintf(stderr, "Chyba kompilatora: zlyhala alokacia pamate\n");
                exit(99);
            }
            str[0] = (char)c;
            str[1] = '\0';
            length = 1;
            state = STATE_IDENTIFIER;
            
            }
            else if (c >= '0' && c <= '9'){
            buffer[0]=c;
            length = 1;
            buffer[length] = '\0';
            state = STATE_NUMBER;

            }

            ////////////////
            else if (c == '+') {
                Ttoken.Ttype = TOKEN_PLUS;
                return Ttoken;
            }
            else if (c == '-') {
                Ttoken.Ttype = TOKEN_MINUS;
                return Ttoken;
            }
            else if (c == '*') {
                Ttoken.Ttype = TOKEN_MUL;
                return Ttoken;
            }
             else if (c == '(') {
                paren_depth++;
                Ttoken.Ttype = TOKEN_LPAREN;
                return Ttoken;
            }
            else if (c == ')') {
                if (paren_depth > 0) {
                    paren_depth--;
                }
                if (paren_depth == 0) {
                    comment_has_newline = false;
                }
                Ttoken.Ttype = TOKEN_RPAREN;
                return Ttoken;
            }
            else if (c == ',') {
                Ttoken.Ttype = TOKEN_COMMA;
                return Ttoken;
            }
            else if (c == ':') {
                Ttoken.Ttype = TOKEN_COLON;
                return Ttoken;
            }
            else if (c == ';') {
                Ttoken.Ttype = TOKEN_SEMICOLON;
                return Ttoken;
            }
            else if (c == '=') {
                state = STATE_EQUALS;
                break;
            }
            else if (c == '<') {
                state = STATE_LESS;
                break;
            }
            else if (c == '>') {
                state = STATE_GREATER;
                break;
            }
            else if (c == '!') {
                state = STATE_NOT;
                break;
            }
            else if (c == '/') {
                state = STATE_SLASH;
                break;
            }
            else if (c == '"') {
                quotes = 1;
                state = STATE_STRING_START;
                break;
            }
            else {
            fprintf(stderr, "Lexikalna chyba 1: neznamy znak\n");
            exit(1);
        }
            break;

        
        case STATE_IDENTIFIER:

            

            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || (c >= '0' && c <= '9')) {
                if (length + 2 > cap) {
                    int newcap = cap > 0 ? cap * 2 : 16;
                    char *tmp = realloc(str, newcap);
                    if (tmp == NULL) {
                        free(str);
                        fprintf(stderr, "Chyba kompilatora: zlyhala alokacia pamate\n");
                        exit(99);
                    }
                    str = tmp;
                    cap = newcap;
                }
                str[length++] = (char)c;
                str[length] = '\0';
                break;
            }
            ungetc(c, source_file);
            if (at_sign) {
                if (strcmp(str, "main") != 0) {
                    free(str);
                    fprintf(stderr, "Lexikalna chyba 1: za @ musi nasledovat main\n");
                    exit(1);
                }
                free(str);
                str = NULL;
                cap = 0;
                length = 0;
                Ttoken.Ttype = TOKEN_AT_MAIN;
                state = STATE_START;
                return Ttoken;
            }

            if (length == 1 && str[0] == '_') {
                free(str);
                fprintf(stderr, "Lexikalna chyba 1: samotne _ nie je identifikator\n");
                exit(1);
            }

            Ttoken.Ttype = lookup_keyword(str);
            if (Ttoken.Ttype == TOKEN_IDENTIFIER) {
                Ttoken.Tatribut.string = str;
                str = NULL;
            }
            else {
                free(str);
            }
            str = NULL;
            cap = 0;
            length = 0;
            state = STATE_START;
            return Ttoken;
            

        case STATE_NUMBER:
        
            if (c == '.') {
                 buf_add(buffer, &length, c);
                state = STATE_DOUBLE;
                break;
            }
            else if (c >= '0' && c <= '9') {
                if (length == 1 && buffer[0] == '0') {
                    fprintf(stderr, "Lexikalna chyba 1: cislo nesmie zacinat nulou\n");
                    exit(1);
                }
                if ((length + 1) > 255) {
                fprintf(stderr, "Lexikalna chyba 1: cislo je prilis dlhe\n");
                    exit(1);
                    break;   
                }
                buf_add(buffer, &length, c);
                
                break;
            }
            else if (c == '_') {
                state = STATE_NUMBER_UNDERSCORE;
                break;                           
            }
            else if (c == 'e' || c == 'E') {
                buf_add(buffer, &length, c);
                state = STATE_EXP;
                break;
            }
            else if (c == 'd' || c == 'D') {
                fprintf(stderr, "Lexikalna chyba 1: pripona d patri len desatinnemu literalu\n");
                exit(1);
            }
            else {
                ungetc(c, source_file);
                Ttoken.Ttype = TOKEN_LITERAL_INT;
                Ttoken.Tatribut.in = atoi(buffer);
                length = 0;
                state = STATE_START;
                return Ttoken;
            }
            
        case STATE_NUMBER_UNDERSCORE:
            if (c >= '0' && c <= '9') {
                if (length == 1 && buffer[0] == '0') {
                    fprintf(stderr, "Lexikalna chyba 1: cislo nesmie zacinat nulou\n");
                    exit(1);
                }
                if ((length + 1) > 255) {
                    fprintf(stderr, "Lexikalna chyba 1: cislo je prilis dlhe\n");
                    exit(1);
                }   
                buf_add(buffer, &length, c);
                state = STATE_NUMBER;
                break;
            }else{
                fprintf(stderr, "Lexikalna chyba 1: neplatny podtrznik v cisle\n");
                exit(1);
            }


        case STATE_EQUALS:
            if (c =='='){
                Ttoken.Ttype=TOKEN_EQ;
                state=STATE_START;
                return Ttoken;
            }
            else{
                ungetc(c, source_file);
                Ttoken.Ttype=TOKEN_ASSIGN;
                state=STATE_START;
                return Ttoken;
            }
        case STATE_LESS:
            if (c == '=') {
                Ttoken.Ttype = TOKEN_LTE;
                state = STATE_START;
                return Ttoken;
            }
            else {
                ungetc(c, source_file);
                Ttoken.Ttype = TOKEN_LT;
                state = STATE_START;
                return Ttoken;
            }

        case STATE_GREATER:
            if (c == '=') {
                Ttoken.Ttype = TOKEN_GTE;
                state = STATE_START;
                return Ttoken;
            }
            else {
                ungetc(c, source_file);
                Ttoken.Ttype = TOKEN_GT;
                state = STATE_START;
                return Ttoken;
            }

        case STATE_NOT:
            if (c == '=') {
                Ttoken.Ttype = TOKEN_NEQ;
                state = STATE_START;
                return Ttoken;
            }
            else {
                fprintf(stderr, "Lexikalna chyba 1: samostatne ! nie je povolene\n");
                exit(1);
            }
        


        case STATE_DOUBLE:                       
            if (c >= '0' && c <= '9') {
                buf_add(buffer, &length, c);
                state = STATE_DOUBLE_DIGITS;
                break;
            }
            fprintf(stderr, "Lexikalna chyba 1: ocakava sa cislica za bodkou\n");
            exit(1);

        case STATE_DOUBLE_DIGITS:                
            if (c >= '0' && c <= '9') {
                buf_add(buffer, &length, c);
                break;
            }
            if (c == '_') {
             state = STATE_DOUBLE_UNDERSCORE;
             break;
            }
            if (c == 'e' || c == 'E') {
                buf_add(buffer, &length, c);
                state = STATE_EXP;
                break;
            }
            if (c == 'd' || c == 'D') {
                Ttoken.Ttype = TOKEN_LITERAL_DOUBLE;
                Ttoken.Tatribut.db = atof(buffer);
                length = 0;
                state = STATE_START;
                return Ttoken;
            }
            ungetc(c, source_file);
            Ttoken.Ttype = TOKEN_LITERAL_DOUBLE;
            Ttoken.Tatribut.db = atof(buffer);
            length = 0;
            state = STATE_START;
            return Ttoken;
            
        case STATE_DOUBLE_UNDERSCORE:
            if (c >= '0' && c <= '9') {
                if ((length + 1) > 255) {
                    fprintf(stderr, "Lexikalna chyba 1: cislo je prilis dlhe\n");
                    exit(1);
                }
                buf_add(buffer, &length, c);
                state = STATE_DOUBLE_DIGITS;
                break;
            }
            fprintf(stderr, "Lexikalna chyba 1: neplatny podtrznik v desatinnej casti\n");
            exit(1);

        case STATE_EXP:                          
            if (c == '+' || c == '-') {
                buf_add(buffer, &length, c);
                state = STATE_EXP_SIGN;
                break;
            }
            if (c >= '0' && c <= '9') {
                buf_add(buffer, &length, c);
                state = STATE_EXP_DIGITS;
                break;
            }
            fprintf(stderr, "Lexikalna chyba 1: ocakava sa cislica v exponente\n");
            exit(1);

        case STATE_EXP_SIGN:               
            if (c >= '0' && c <= '9') {
                buf_add(buffer, &length, c);
                state = STATE_EXP_DIGITS;
                break;
            }
            fprintf(stderr, "Lexikalna chyba 1: ocakava sa cislica v exponente\n");
            exit(1);

        case STATE_EXP_DIGITS:                  
            if (c >= '0' && c <= '9') {
                buf_add(buffer, &length, c);
                break;
            }
            if (c == '_') {
            state = STATE_EXP_UNDERSCORE;
            break;
            }

            if (c == 'd' || c == 'D') {
                Ttoken.Ttype = TOKEN_LITERAL_DOUBLE;
                Ttoken.Tatribut.db = atof(buffer);
                length = 0;
                state = STATE_START;
                return Ttoken;
            }
            ungetc(c, source_file);
            Ttoken.Ttype = TOKEN_LITERAL_DOUBLE;
            Ttoken.Tatribut.db = atof(buffer);
            length = 0;
            state = STATE_START;
            return Ttoken;

        case STATE_EXP_UNDERSCORE:
            if (c >= '0' && c <= '9') {
                if ((length + 1) > 255) {
                    fprintf(stderr, "Lexikalna chyba 1: cislo je prilis dlhe\n");
                    exit(1);
                }
                buf_add(buffer, &length, c);
                state = STATE_EXP_DIGITS;
                break;
            }
            fprintf(stderr, "Lexikalna chyba 1: neplatny podtrznik v exponente\n");
            exit(1);

        case STATE_SLASH:
            if (c == '/') {
                state = STATE_LINE_COMMENT;
                break;
            }
            else if (c == '*') {
                comment_depth = 1;
                state = STATE_BLOCK_COMMENT;
                break;
            }
            else {
                if (paren_depth == 0 && comment_has_newline) {
                    fprintf(stderr, "Lexikalna chyba 1: za blokovym komentarom s novym riadkom nesmie byt kod\n");
                    exit(1);
                }
                ungetc(c, source_file);
                Ttoken.Ttype = TOKEN_DIV;
                state = STATE_START;
                return Ttoken;
            }

        case STATE_LINE_COMMENT:
            if (c == '\n') {
                ungetc(c, source_file);
                state = STATE_START;
            }
            else if (c == EOF) {
                state = STATE_START;
            }
            break;
            
        case STATE_BLOCK_COMMENT:
            if (c == '\n') {
                comment_has_newline = true;
                state = STATE_BLOCK_COMMENT;
            }

            else if (c == '/') {
                state = STATE_BLOCK_COMMENT_SLASH;
            }
            else if (c == '*') {
                state = STATE_BLOCK_COMMENT_STAR;
            }
            else if (c == EOF) {
                fprintf(stderr, "Lexikalna chyba 1: neukonceny blokovy komentar\n");
                exit(1);
            }
            break;

        case STATE_BLOCK_COMMENT_SLASH:
             if (c == '\n') {
                comment_has_newline = true;
                state = STATE_BLOCK_COMMENT;
            }

            else if (c == '*') {
                comment_depth++;
                state = STATE_BLOCK_COMMENT;
            }
            else if (c == '/') {
                state = STATE_BLOCK_COMMENT_SLASH;
            }
            else if (c == EOF) {
                fprintf(stderr, "Lexikalna chyba 1: neukonceny blokovy komentar\n");
                exit(1);
            }
            else {
                state = STATE_BLOCK_COMMENT;
            }
            break;

        case STATE_BLOCK_COMMENT_STAR:
            if (c == '\n') {
                comment_has_newline = true;
                state = STATE_BLOCK_COMMENT;
            }

            else if (c == '/') {
                comment_depth--;
                if (comment_depth == 0) {
                    if (comment_from_paren) {
                        comment_from_paren = false;
                        state = STATE_PAREN_LINE_START;
                    }
                    else {
                        state = STATE_START;
                    }
                }
                
                else {
                    state = STATE_BLOCK_COMMENT;
                }
            }
            else if (c == '*') {
                state = STATE_BLOCK_COMMENT_STAR;
            }
            else if (c == EOF) {
                fprintf(stderr, "Lexikalna chyba 1: neukonceny blokovy komentar\n");
                exit(1);
            }
            else {
                state = STATE_BLOCK_COMMENT;
            }
            break;
        
            case STATE_STRING_START:
            if (c == '"') {
                quotes++;
                state = STATE_STRING_QUOTES2;
            }
            else {
                ungetc(c, source_file);
                cap = 16;
                str = malloc(cap);
                if (str == NULL) {
                    fprintf(stderr, "Chyba kompilatora: zlyhala alokacia pamate\n");
                    exit(99);
                }
                length = 0;
                str[0] = '\0';
                state = STATE_STRING;
            }
            break;


           case STATE_STRING_QUOTES2:
            if (c == '"') {
                quotes = 3;
                cap = 16;
                str = malloc(cap);
                if (str == NULL) {
                    fprintf(stderr, "Chyba kompilatora: zlyhala alokacia pamate\n");
                    exit(99);
                }
                length = 0;
                str[0] = '\0';
                state = STATE_MULTILINE_STRING;
                break;
            }

            //prazdny string
            ungetc(c, source_file);
            
                char *s = malloc(1);
                if (s == NULL) {
                    fprintf(stderr, "Chyba kompilatora: zlyhala alokacia pamate\n");
                    exit(99);
                }
                s[0] = '\0';
                Ttoken.Ttype = TOKEN_LITERAL_STRING;
                Ttoken.Tatribut.string = s;
            
                quotes = 0;
                state = STATE_START;
                return Ttoken;



            case STATE_MULTILINE_STRING:
                if (c == '"') {
                    quotes = 1;
                    state = STATE_MULTILINE_QUOTES;
                    break;
                }
                if (c == EOF) {
                    free(str);
                    fprintf(stderr, "Lexikalna chyba 1: neukonceny viacriadkovy retazec\n");
                    exit(1);
                }
                if (length + 2 > cap) {
                    int newcap = cap * 2;
                    char *tmp = realloc(str, newcap);
                    if (tmp == NULL) {
                        free(str);
                        fprintf(stderr, "Chyba kompilatora: zlyhala alokacia pamate\n");
                        exit(99);
                    }
                    str = tmp;
                    cap = newcap;
                }
                str[length++] = (char)c;
                str[length] = '\0';
                break;




            case STATE_MULTILINE_QUOTES:
                if (c == '"') {
                    quotes++;
                    break;
                }
                if (quotes >= 3) {
                    //posledne tri uvodzovky ukoncuju ostatne patria do obsahu 
                    int extra = quotes - 3;
                    while (length + extra + 2 > cap) {
                        int newcap = cap * 2;
                        char *tmp = realloc(str, newcap);
                        if (tmp == NULL) {
                            free(str);
                            fprintf(stderr, "Chyba kompilatora: zlyhala alokacia pamate\n");
                            exit(99);
                        }
                        str = tmp;
                        cap = newcap;
                    }
                    for (int i = 0; i < extra; i++)
                        str[length++] = '"';
                    str[length] = '\0';

                    Ttoken.Ttype = TOKEN_LITERAL_STRING;
                    Ttoken.Tatribut.string = str;
                    str = NULL;
                    cap = 0;
                    length = 0;
                    quotes = 0;
                    state = STATE_START;
                    if (c != EOF)
                        ungetc(c, source_file);
                    return Ttoken;
                }
                // menej nez tri uvodzovky sucast obsahu 
                if (c == EOF) {
                    free(str);
                    fprintf(stderr, "Lexikalna chyba 1: neukonceny viacriadkovy retazec\n");
                    exit(1);
                }
                while (length + quotes + 2 > cap) {
                    int newcap = cap * 2;
                    char *tmp = realloc(str, newcap);
                    if (tmp == NULL) {
                        free(str);
                        fprintf(stderr, "Chyba kompilatora: zlyhala alokacia pamate\n");
                        exit(99);
                    }
                    str = tmp;
                    cap = newcap;
                }
                for (int i = 0; i < quotes; i++) str[length++] = '"';
                quotes = 0;
                str[length] = '\0';
                ungetc(c, source_file);
                state = STATE_MULTILINE_STRING;
                break;

            case STATE_STRING:
                if (c == '"') {
                Ttoken.Ttype = TOKEN_LITERAL_STRING;
                Ttoken.Tatribut.string = str;
                str = NULL;
                cap = 0;
                length = 0;
                state = STATE_START;
                return Ttoken;
            }
            if (c == '\n' || c == EOF) {
                free(str);
                fprintf(stderr, "Lexikalna chyba 1: neukonceny retazec\n");
                exit(1);
            }
            if (c == '\\') {
                state = STATE_STRING_ESCAPE;
                break;
            }
            if (length + 2 > cap) {      
                int newcap = cap * 2;
                char *tmp = realloc(str, newcap);
                if (tmp == NULL) {
                    free(str);
                    fprintf(stderr, "Chyba kompilatora: zlyhala alokacia pamate\n");
                    exit(99);
                }
                str = tmp;
                cap = newcap;
            }
                str[length++] = (char)c;
                str[length] = '\0';
                break;


            // escape sekvence
            case STATE_STRING_ESCAPE: {
                char out;
                if (c == 'n')       out = '\n';
                else if (c == 't')  out = '\t';
                else if (c == 'r')  out = '\r';
                else if (c == '\\') out = '\\';
                else if (c == '"')  out = '"';

                else {
                    free(str);
                    fprintf(stderr, "Lexikalna chyba 1: neplatna escape sekvencia\n");
                    exit(1);
                }


                if (length + 2 > cap) {
                    int newcap = cap * 2;
                    char *tmp = realloc(str, newcap);
                    if (tmp == NULL) {
                        free(str);
                        fprintf(stderr, "Chyba kompilatora: zlyhala alokacia pamate\n");
                        exit(99);
                    }
                    str = tmp;
                    cap = newcap;
                }
                str[length++] = out;
                str[length] = '\0';
                state = STATE_STRING;
                break;
            }
            case STATE_DOT:
                if (c >= '0' && c <= '9') {
                    buf_add(buffer, &length, c);
                    state = STATE_DOUBLE_DIGITS;
                    break;
                }
                ungetc(c, source_file);
                length = 0;
                Ttoken.Ttype = TOKEN_DOT;
                state = STATE_START;
                return Ttoken;


            case STATE_PAREN_LINE_START:

            if (c == ' ') {
                space_count++;
                break;
            }
            if (c == '\n' || c == '\r') {
                tab_in_indent = false;
                space_count = 0;
                break;
            }
            if (c == '\t') {
                tab_in_indent = true;
                break;
            }
            if (c == EOF) {
                space_count = 0;
                tab_in_indent = false;
                state = STATE_START;
                break;
            }
            if (c == '/') {
                int next = fgetc(source_file);           
                if (next == '/') {                       
                    while ((c = fgetc(source_file)) != '\n' && c != EOF);   
                    ungetc(c, source_file);              
                    break;
                }
                if (next == '*') {                  
                    comment_depth = 1;
                    comment_from_paren = true;       
                    state = STATE_BLOCK_COMMENT;
                    break;
                }
                ungetc(next, source_file);               
            }
            ungetc(c, source_file);
            if (tab_in_indent) {
                fprintf(stderr, "Lexikalna chyba 1: Tabulator v odsadeni\n");
                exit(1);
            }
            if (c != ')' && space_count <= indent_stack[indent_top]) {
                fprintf(stderr, "Syntakticka chyba 2: pokracovaci riadok v zatvorkach musi byt odsadeny viac\n");
                exit(2);
            }
            space_count = 0;
            state = STATE_START;
            break;


        default:
            break;
       
        }
    }
    

}