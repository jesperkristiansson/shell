#include "io.h"
#include "globals.h"
#include "error.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define DEF_PROMPT "> "

static int starts_with(char *full_str, char *start){
    int i;
    for(i = 0; start[i] != '\0'; ++i){
        if(full_str[i] != start[i]){
            return -1;
        }
    }
    return i;
}

static void print_prompt(){
    char cwd[MAXBUF];
    char *to_print = cwd;
    getcwd(cwd, MAXBUF);
    int i;
    if((i = starts_with(cwd, getenv("HOME"))) >= 0){
        to_print = &to_print[i-1];
        to_print[0] = '~';
    }
    printf("\033[1;34m%s\033[0m%s", to_print, DEF_PROMPT);
    fflush(stdout);
}

int fetch_line(char *str_ptr){
    int i = 0;
    int c;
    print_prompt();
    str_pos = 0;
    token_pos = 0;
    while((c = getchar())){
        if(c == EOF){
            putchar('\n');
            return EOF;
        } else if(i < MAXBUF-1){
            str_ptr[i++] = (char) c;
        }
        if(c == '\n'){
            break;
        }
    }
    if(i >= MAXBUF-1){
        print_error("Too many tokens\n");
        return fetch_line(str_ptr);
    }
    str_ptr[i++] = '\0';
    return i;
}