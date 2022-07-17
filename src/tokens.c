#include "tokens.h"
#include "globals.h"
#include "error.h"

bool end_of_token(char c){
    switch (c){
        case '\0':
        case ' ':
        case '\t':
        case ';':
        case '>':
        case '<':
        case '|':
        case '&':
        case '\"':
            return true;
        default:
            return false;
    }
}

token_t get_token(char *str, char **token_ptr){
    static char token_buf[MAXBUF+MAXARGS];
    token_t type;
    while(str[str_pos] == ' ' || str[str_pos] == '\t'){
        str_pos++;
    }
    *token_ptr = &token_buf[token_pos];
    switch (str[str_pos]){
        case ';':
            ++str_pos;
            type = SEMICOLON;
            break;
        case '>': 
            ++str_pos;
            type = OUTPUT;
            break;
        case '<':
            ++str_pos;
            type = INPUT;
            break;
        case '|':
            ++str_pos;
            type = PIPE;
            break;
        case '&':
            ++str_pos;
            type = AMPERSAND;
            break;
        case '\0':
            type = NULLBYTE;
            break;
        case '\"':
            type = RAW;
            while(str[++str_pos] != '\"'){
                if(str_pos >= MAXBUF){
                    print_error("No closing \"\n");
                    return -1;
                }
                token_buf[token_pos++] = str[str_pos];
            }
            ++str_pos;
            break;
        default:
            type = NORMAL;
            while(!end_of_token(str[str_pos])){
                token_buf[token_pos++] = str[str_pos++];
            }
            break;
    }
    token_buf[token_pos++] = '\0';
    return type;
}