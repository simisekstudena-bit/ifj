#include "scanner.h"

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
            
        


        case STATE_DOUBLE:
            fprintf(stderr, "Lexikalna chyba 1: desatinne cisla zatial nepodporovane\n");
            exit(1);



        default:
            break;
       
        }
    }
    

}