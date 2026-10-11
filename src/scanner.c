#include "scanner.h"
static int comment_depth = 0;
static ScannerState state = STATE_LINE_START;
static int last_indent = 0;

Token get_token(FILE *source_file){
    Token Ttoken;
    int c;
    char buffer[256];
    int length = 0;
    
    int space_count = 0;

    

    while (true){
    
    c=fgetc(source_file);

    switch (state){

        case STATE_LINE_START:

           
            if (c == EOF){
            Ttoken.Ttype = TOKEN_EOF;
            return Ttoken;

                }

         
            else if (c == ' '){
                space_count ++;

            }

            
            else if (c == '\t'){
                fprintf(stderr, "Lexikalna chyba 1: Tabulator v odsadeni\n");
                exit(1);


            }
            else if (c == '\n') {
                space_count = 0;
                continue;
            }else if (c == '/') {
                int next = fgetc(source_file);
                if (next == '/') {
                    //riadok je komentar ignorovany cely riadok 
                    while ((c = fgetc(source_file)) != '\n' && c != EOF);
                    space_count = 0;
                    continue;
                }
                ungetc(next, source_file);
                ungetc(c, source_file);
            }
            else{

            
            ungetc(c, source_file);


            //ident 
            if (last_indent < space_count){
                last_indent=space_count;
                Ttoken.Ttype=TOKEN_INDENT;
                Ttoken.Tatribut.in =space_count;
                state = STATE_START;
                return Ttoken;
            }

            //dedent
            if (last_indent > space_count){
                last_indent=space_count;
                Ttoken.Ttype=TOKEN_DEDENT;
                Ttoken.Tatribut.in =space_count;
                state = STATE_START;
                return Ttoken;
            }
            space_count = 0;
            state = STATE_START;

            }
            break;


        case STATE_START:
            
            if (c == ' ' || c == '\t')break;
             
            else if (c == '\n' || c == '\r') {
                Ttoken.Ttype = TOKEN_EOL;
                state = STATE_LINE_START;
                return Ttoken;
            }

            else if (c == EOF) {
                Ttoken.Ttype = TOKEN_EOF;
                return Ttoken;
            }
            else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
            
            buffer[0]=c;
            length = 1;
            buffer[length] = '\0';
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
                Ttoken.Ttype = TOKEN_LPAREN;
                return Ttoken;
            }
            else if (c == ')') {
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
            else {
            fprintf(stderr, "Lexikalna chyba 1: neznamy znak\n");
            exit(1);
        }
            break;

            
        case STATE_IDENTIFIER:
            if( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || (c >= '0' && c <= '9')){
                if ((length + 1 )>255) break;//TODO ERROR VYRIESIT 
                buffer[length++] = c;
                buffer[length]= '\0';
                break;
            }
            else {
                ungetc(c, source_file);
                //TODO: TABULKU POROVNAVANIA IDENTIF S SLOVAMI IF,def,.....


                Ttoken.Ttype = TOKEN_IDENTIFIER;
                //TODO: vyplnenie stringu v atribute tokenu 

                //Ttoken.Tatribut.string =;


                length = 0;
                state = STATE_START;
                return Ttoken;
            
            }

        case STATE_NUMBER:
            if (c == '.') {
                 buffer[length++] = c;
                buffer[length] = '\0';
                state = STATE_DOUBLE;
                break;
            }
            else if (c >= '0' && c <= '9') {
                if ((length + 1) > 255) break;   // todo chyba pri pretečení
                buffer[length++] = c;
                buffer[length] = '\0';
                break;
            }
            else if (c == '_') {
                state = STATE_NUMBER_UNDERSCORE;
                break;                           
            }
            else if (c == 'e' || c == 'E') {
                buffer[length++] = c;
                buffer[length] = '\0';
                state = STATE_EXP;
                break;
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
                if ((length + 1) > 255) {
                    fprintf(stderr, "Lexikalna chyba 1: cislo je prilis dlhe\n");
                    exit(1);
                }   
                buffer[length++] = c;
                buffer[length] = '\0';
                state = STATE_NUMBER;
                break;
            }else{
                fprintf(stderr, "Lexikalna chyba 1: neplatny podtrznik v cisle\n");
                exit(1);
                //TODO ERROR
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
                buffer[length++] = c;
                buffer[length] = '\0';
                state = STATE_DOUBLE_DIGITS;
                break;
            }
            fprintf(stderr, "Lexikalna chyba 1: ocakava sa cislica za bodkou\n");
            exit(1);

        case STATE_DOUBLE_DIGITS:                
            if (c >= '0' && c <= '9') {
                buffer[length++] = c;
                buffer[length] = '\0';
                break;
            }
            if (c == 'e' || c == 'E') {
                buffer[length++] = c;
                buffer[length] = '\0';
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

        case STATE_EXP:                          
            if (c == '+' || c == '-') {
                buffer[length++] = c;
                buffer[length] = '\0';
                state = STATE_EXP_SIGN;
                break;
            }
            if (c >= '0' && c <= '9') {
                buffer[length++] = c;
                buffer[length] = '\0';
                state = STATE_EXP_DIGITS;
                break;
            }
            fprintf(stderr, "Lexikalna chyba 1: ocakava sa cislica v exponente\n");
            exit(1);

        case STATE_EXP_SIGN:               
            if (c >= '0' && c <= '9') {
                buffer[length++] = c;
                buffer[length] = '\0';
                state = STATE_EXP_DIGITS;
                break;
            }
            fprintf(stderr, "Lexikalna chyba 1: ocakava sa cislica v exponente\n");
            exit(1);

        case STATE_EXP_DIGITS:                  
            if (c >= '0' && c <= '9') {
                buffer[length++] = c;
                buffer[length] = '\0';
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
            if (c == '/') {
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
            if (c == '*') {
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
            if (c == '/') {
                comment_depth--;
                if (comment_depth == 0) {
                    state = STATE_START;
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


        default:
            break;
       
        }
    }
    

}