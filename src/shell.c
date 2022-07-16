#include "io.h"
#include "globals.h"
#include "execute.h"
#include "quit.h"
#include <stdio.h>

void init(){
    init_terminal();
}

int main(){
    init();

    char input[MAXBUF];
    while(fetch_line(input) != EOF){
        parse_line(input);
    }
    quit();
    return 0;
}