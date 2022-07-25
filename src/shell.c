#include "io.h"
#include "globals.h"
#include "execute.h"
#include "quit.h"
#include "command_history.h"
#include <stdio.h>
#include <unistd.h>

void init(){
    ch_init();
}

int main(int argc, char **argv){
    char input[MAXBUF];
    if(argc == 1){  //get input from stdin
        init();

        while(fetch_line(input) != EOF){
            parse_line(input);
        }
        quit(0);
    } else if(argc == 2){   //try to run the file argv[1] as a script
        if(access(argv[1], R_OK) == 0){
            FILE *fp = fopen(argv[1], "r");

            while(fetch_line_file(input, fp) != EOF){
                parse_line(input);
            }
            fclose(fp);
        } else{
            perror(argv[1]);
            return -1;
        }
    } else{
        printf("Too many arguments\n");
        return -1;
    }
    return 0;
}