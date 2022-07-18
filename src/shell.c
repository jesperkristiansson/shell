#include "io.h"
#include "globals.h"
#include "execute.h"
#include "quit.h"
#include <stdio.h>

void init(){
    init_terminal();
    get_terminal_size();
}

int main(){
    init();

    char input[MAXBUF];
    while(fetch_line(input) != EOF){
        str_pos = 0;
        token_pos = 0;
        parse_line(input);
    }
    quit(0);
    return 0;
}