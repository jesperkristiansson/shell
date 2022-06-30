#include <stdio.h>

#define MAXBUF 16
#define PROMPT "> "

void error(const char *msg){
    printf("%s", msg);
}

int fetch_line(char *str_ptr){  //improve handling of overflow 
    int i = 0;
    int c;
    printf(PROMPT);
    while((c = getchar()) != '\n'){
        if(c == EOF){
            putchar('\n');
            return EOF;
        } else if(i < MAXBUF-1){
            str_ptr[i++] = (char) c;
        }
    }
    if(i >= MAXBUF-1){
        error("Too many tokens\n");
        return fetch_line(str_ptr);
    }
    str_ptr[i++] = '\0';
    return i;
}

void parse_line(char *str_ptr){
    printf("%s\n", str_ptr);
}

int main(int argc, char **argv){
    char input[512];
    while(fetch_line(input) != EOF){
        parse_line(input);
    }
    return 0;
}