#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>

#define MAXBUF 512
#define MAXARGS 128
#define MAXTOKSIZE 64 
#define PROMPT "> "

void init(){
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
        case '\"':
            return true;
        default:
            return false;
    }
}

void error(const char *msg){
    printf("%s", msg);
}

static char *current_directory(){
    static char current_dir[MAXBUF];
    getcwd(current_dir, MAXBUF);
    return current_dir;
}

int fetch_line(char *str_ptr){
    int i = 0;
    int c;
    printf("\033[1;34m%s\033[0m%s",current_directory(), PROMPT);
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

int tokenize(char *str, char **tokens_ptr){
    static char token_buf[MAXBUF+MAXARGS]; //max sife of input + a '\0' for every token
    int i = 0;
    int token_i = 0;
    for(int pos = 0; pos < MAXBUF;){
        while(str[pos] == ' ' || str[pos] == '\t'){
            pos++;
        }
        switch (str[pos]){
            case ';':
            case '>':
            case '<':
            case '|':
            case '&':
                tokens_ptr[i] = &token_buf[token_i];
                token_buf[token_i++] = str[pos++];
                break;
            case '\0':
                return i;
            case '\"':
                tokens_ptr[i] = &token_buf[token_i];
                while(str[++pos] != '\"'){
                    if(str[pos] == '\0'){
                        error("No closing \"\n");
                        return -1;
                    }
                    token_buf[token_i++] = str[pos++];
                }
                ++pos;
                break;
            default:
                tokens_ptr[i] = &token_buf[token_i];
                while(!end_of_token(str[pos])){
                    token_buf[token_i++] = str[pos++];
                }
                break;
        }
        token_buf[token_i++] = '\0';
        i++;
    }
    error("too long input\n");
    return -1;
}

void cd(int argc, char **argv){
    char dest[MAXBUF];
    static char prev_dir[MAXBUF] = "";
    if(argc == 1){
        strncpy(dest, getenv("HOME"), MAXBUF);
    } else if(argc == 2){
        if(argv[1][0] == '-'){
            if(prev_dir[0] == '\0'){
                error("No previous directory\n");
                return;
            }
            strncpy(dest, prev_dir, MAXBUF);
        } else{
            getcwd(dest, MAXBUF);
            strcat(dest, "/");
            strncat(dest, argv[1], MAXBUF-strlen(dest)-strlen(argv[1]));
        }
    } else{
        error("Too many arguments to cd\n");
        return;
    }
    char *cwd = current_directory();
    if(chdir(dest)){
        error("Invalid path\n");
    } else{
        strncpy(prev_dir, cwd, MAXBUF);
    }
}

void run_program(int argc, char **argv, bool foreground){
    if(strncmp(argv[0], "cd", 3) == 0){
        cd(argc, argv);
        return;
    }
    pid_t pid = fork();
    if(pid == 0){
        if(execvp(argv[0], argv)){
            printf("%s\n", getenv("PATH"));
            error("execvp failed\n");
            printf("%s\n", strerror(errno));
            exit(-1);
        }
    }
    if(pid < 0){
        error("fork failed\n");
        return;
    }
    if(foreground){
        int wstatus;
        waitpid(pid, &wstatus, 0);
    }
}

void execute(int argc, char **tokens){
    char *argv[argc+1];
    for(int i = 0; i < argc; i++){
        argv[i] = tokens[i];
    }
    argv[argc] = NULL;
    run_program(argc, argv, true);
}

int main(int argc, char **argv){
    init();

    char input[MAXBUF];
    char *tokens[MAXARGS];
    while(fetch_line(input) != EOF){
        int num = tokenize(input, tokens);
        #ifdef DEBUG
            printf("\033[0;33m");   //yellow for debug
            printf("Tokens generated:\n");
            for(int i = 0; i < num; i++){
                printf("%s\n", tokens[i]);
            }
            printf("\033[0m");
            fflush(stdout);
        #endif
        if(num){
            execute(num, tokens);
        }
    }
    return 0;
}