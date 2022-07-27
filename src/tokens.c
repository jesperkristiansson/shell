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
        case SEMICOLON:
        case OUTPUT: 
        case INPUT:
        case PIPE:
        case AMPERSAND:
        case NULLBYTE:
            type = str[str_pos++];
            break;
        case RAW:
            type = RAW;
            while(str[++str_pos] != '\"'){
                if(str_pos >= MAXBUF || str[str_pos] == '\0'){
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