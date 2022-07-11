#include "io.h"
#include "globals.h"
#include <stdio.h>

int main(){
    char input[MAXBUF];
    while(fetch_line(input) != EOF){
        parse_line(input);
    }
    return 0;
}