#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXBUF 512
#define MAXARGS 100
#define PROMPT "> "

static char *input_pos;

void init_path(){

}

bool end_of_token(char c){
    switch (c){
        case '\0':
        case ' ':
        case '\n':
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

/*int get_token(char *str_ptr, char **token_ptr){
    while(*input_pos == ' ' || *input_pos == '\t'){
        input_pos++;
    }
    while(!end_of_token(*input_pos)){
        *token_ptr++ = *input_pos++;
    }
    return 0;
}
*/

void error(const char *msg){
    printf("%s", msg);
}

int fetch_line(char *str_ptr){
    int i = 0;
    int c;
    input_pos = str_ptr;
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

void run_program(int argc, char **argv, bool foreground){
    pid_t pid = fork();
    if(pid == 0){
        execvp(argv[0], argv);
    }
    if(foreground){
        int wstatus;
        waitpid(pid, &wstatus, 0);
    }
}

void parse_line(char *str_ptr){
    char *argv[MAXARGS];
    //get_token(str_ptr, &argv[0]);
    if(strncmp("ls", str_ptr, 3) == 0){
        argv[0] = "ls";
        argv[1] = NULL;
        run_program(1, argv, 1);
    }
}

int main(int argc, char **argv){
    init_path();

    char input[MAXBUF];
    while(fetch_line(input) != EOF){
        parse_line(input);
    }
    return 0;
}