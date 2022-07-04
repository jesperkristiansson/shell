#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#define MAXBUF 512
#define MAXARGS 128
#define MAXTOKSIZE 64 
#define DEF_PROMPT "> "
#define PRINT_ERROR printf("%s\n", strerror(errno))
#define DEF_PERM 0644

static int str_pos;
static int token_pos;

typedef enum{
    INPUT,
    OUTPUT,
    PIPE,
    NEWLINE,
    SEMICOLON,
    AMPERSAND,
    VAR,
    NORMAL
} token_t;

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

char *current_directory(){
    static char current_dir[MAXBUF];
    getcwd(current_dir, MAXBUF);
    return current_dir;
}

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
        error("Too many tokens\n");
        return fetch_line(str_ptr);
    }
    str_ptr[i++] = '\0';
    return i;
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
            token_buf[token_pos++] = str[str_pos++];
            type = SEMICOLON;
            break;
        case '>': 
            token_buf[token_pos++] = str[str_pos++];
            type = OUTPUT;
            break;
        case '<':
            token_buf[token_pos++] = str[str_pos++];
            type = INPUT;
            break;
        case '|':
            token_buf[token_pos++] = str[str_pos++];
            type = PIPE;
            break;
        case '\n':
            token_buf[token_pos++] = str[str_pos++];
            type = NEWLINE;
            break;
        case '&':
            token_buf[token_pos++] = str[str_pos++];
            type = AMPERSAND;
            break;
        case '\"':
            type = NORMAL;
            while(str[++str_pos] != '\"'){
                if(str_pos >= MAXBUF){
                    error("No closing \"\n");
                    return -1;
                }
                token_buf[token_pos++] = str[str_pos];
            }
            ++str_pos;
            break;
        case '$':       //add support for $(cmd) to execute cmd and use the output as a token
            type = VAR;
            ++str_pos;
            if(str[str_pos] == '{'){
                while(str[++str_pos] != '}'){
                    token_buf[token_pos++] = str[str_pos];
                }
                ++str_pos;
            } else{
                 while(!end_of_token(str[str_pos])){
                    token_buf[token_pos++] = str[str_pos++];
                }
            }
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
        } else if(argv[1][0] == '/'){
            strcpy(dest, argv[1]);
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

void help(){
    error("help-command not yet implemented\n");
}

bool check_builtins(int argc, char **argv){
    char *progname = argv[0];
    if(strcmp(progname, "cd") == 0){
        cd(argc, argv);
        return true;
    } else if(strcmp(progname, "help") == 0){
        help();
        return true;
    } else if(strcmp(progname, "exit") == 0){
        exit(0);
    }
    return false;
}

void run_program(int argc, char **argv, bool foreground, int input_fd, int output_fd){
    if(check_builtins(argc, argv)){
        return;
    }
    pid_t pid = fork();
    if(pid == 0){
        dup2(input_fd, STDIN_FILENO);
        dup2(output_fd, STDOUT_FILENO);
        if(execvp(argv[0], argv)){
            PRINT_ERROR;
            exit(-1);
        }
    }
    if(pid < 0){
        PRINT_ERROR;
        return;
    }
    if(foreground){
        int wstatus;
        waitpid(pid, &wstatus, 0);
    } else{
        printf("Background process started with pid: %d\n", pid);
    }
}

void parse_line(char *input){
    char *tokens[MAXARGS];
    token_t type;
    int argc = 0;
    bool foreground = 1;
    bool doing_pipe = false;
    int input_fd = STDIN_FILENO;
    int output_fd = STDOUT_FILENO;
    int pipe_fd[2];
    for(;;){
        type = get_token(input, &tokens[argc]);
        switch (type){
            case VAR:
                tokens[argc] = getenv(tokens[argc]);
            case NORMAL:
                argc++;
                break;
            case INPUT:
                type = get_token(input, &tokens[argc]);
                if(type != NORMAL){
                    printf("Expected filename but got %s\n", tokens[argc]);
                    return;
                }
                input_fd = open(tokens[argc], O_RDONLY);
                if(input_fd < 0){
                    printf("Cannot read from %s", tokens[argc]);
                    return;
                }
                break;
            case OUTPUT:
                type = get_token(input, &tokens[argc]);
                if(type != NORMAL){
                    printf("Expected filename but got %s\n", tokens[argc]);
                    return;
                }
                output_fd = open(tokens[argc], O_WRONLY | O_CREAT, DEF_PERM);
                if(output_fd < 0){
                    printf("Cannot write to %s", tokens[argc]);
                    return;
                }
                break;
            case PIPE:
                pipe(pipe_fd);
                output_fd = pipe_fd[1];
                doing_pipe = true;
            case AMPERSAND:
                foreground = 0;
            case NEWLINE:
            case SEMICOLON:
                if(argc == 0){
                    return;
                }
                if(strcmp(tokens[0], "ls") == 0){
                    tokens[argc++] = "--color=auto";
                }   
                tokens[argc] = NULL;
                fflush(stdout);
                run_program(argc, tokens, foreground, input_fd, output_fd);
                argc = 0;
                foreground = 1;
                if(input_fd != STDIN_FILENO){
                    close(input_fd);
                    input_fd = STDIN_FILENO;
                }
                if(output_fd != STDOUT_FILENO){
                    close(output_fd);
                    output_fd = STDOUT_FILENO;
                }
                if(doing_pipe){
                    input_fd = pipe_fd[0];
                    doing_pipe = false;
                }
                if(type == NEWLINE){
                    return;
                }
                break;
            default:
                error("No matching token type\n");
        }
    }
}

int main(int argc, char **argv){
    init();

    char input[MAXBUF];
    while(fetch_line(input) != EOF){
        parse_line(input);
        waitpid(-1, NULL, WNOHANG);
    }
    return 0;
}