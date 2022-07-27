#include "execute.h"
#include "globals.h"
#include "tokens.h"
#include "error.h"
#include "quit.h"
#include "command_history.h"
#include "alias.h"
#include <wordexp.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

#define DEF_PERM 0644

void cd(int argc, char **argv){
    char dest[MAXBUF];
    static char prev_dir[MAXBUF] = "";
    if(argc == 1){
        strncpy(dest, getenv("HOME"), MAXBUF);
    } else if(argc == 2){
        if(argv[1][0] == '-'){
            if(prev_dir[0] == '\0'){
                print_error("No previous directory\n");
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
        print_error("Too many arguments to cd\n");
        return;
    }
    char cwd[MAXBUF];
    getcwd(cwd, MAXBUF);
    if(chdir(dest)){
        print_error("Invalid path\n");
    } else{
        strncpy(prev_dir, cwd, MAXBUF);
    }
}

void help(){
    print_error("help-command not yet implemented\n");
}

void set_env_var(int argc, char **argv){
    if(argc < 2){
        print_error("export doesn't support less than 2 arguments\n");
        return;
    }
    for(int n = 1; n < argc; n++){
        char *arg = argv[n]; 
        int i = 0;
        while(arg[i] != '='){
            if(arg[i++] == '\0'){
                printf("invalid assigment at position %d", n);
                continue;
            }
        }
        arg[i++] = '\0';
        setenv(arg, &arg[i], 1);
    }
}

void print_history(){
    char **history;
    int history_size = get_history(&history);
    for(int i = 0; i < history_size; ++i){
        printf("%5d  %s\n", i+1, history[i]);
    }
}

void alias(int argc, char **argv){
    if(argc == 1){
        string_map_node *current = get_alias_list()->head;
        while(current){
            printf("%s='%s'\n", current->key, current->val);
            current = current->next;
        }
    } else{
        for(int i = 1; i < argc; ++i){      //if it follows key=value, assign it
            char *val_index;
            for(val_index = argv[i]; *val_index != '\0' && *val_index != '='; ++val_index);     //step val_index forward until it passes the first '='
            if(*val_index == '='){
                *val_index++ = '\0';
                set_alias(argv[i], val_index);
            } else{
                char *cmd = get_alias(argv[i]);
                if(cmd){
                    printf("%s='%s'\n", argv[i], cmd);
                } else{
                    printf("%s: %s: not found\n", argv[0], argv[i]);
                }
            }
        }
    }
}

void unalias(int argc, char **argv){
    if(argc > 1){
        for(int i = 1; i < argc; ++i){
            if(!unset_alias(argv[i])){
                printf("%s: %s: not found\n", argv[0], argv[i]);
            }
        }
    }
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
        quit(0);
    } else if(strcmp(progname, "export") == 0){
        set_env_var(argc, argv);
        return true;
    }else if(strcmp(progname, "history") == 0){
        print_history();
        return true;
    } else if(strcmp(progname, "alias") == 0){
        alias(argc, argv);
        return true;
    } else if(strcmp(progname, "unalias") == 0){
        unalias(argc, argv);
        return true;
    }
    return false;
}

void run_program(int argc, char **argv, bool foreground, int input_fd, int output_fd, bool doing_pipe){
    if(check_builtins(argc, argv)){
        return;
    }
    pid_t pid = fork();
    if(pid == 0){
        dup2(input_fd, STDIN_FILENO);
        dup2(output_fd, STDOUT_FILENO);
        if(execvp(argv[0], argv)){
            perror(argv[0]);
            exit(-1);
        }
    }
    if(pid < 0){
        perror("fork");
        return;
    }
    if(foreground){
        int wstatus;
        waitpid(pid, &wstatus, 0);
    } else if(!doing_pipe){
        printf("Background process started with pid: %d\n", pid);
    }
}

/* Should be changed to evaluate the whole input before starting programs to make pipes work properly */
void parse_line(char *input){
    str_pos = 0;
    token_pos = 0;
    waitpid(-1, NULL, WNOHANG);
    char *tokens[MAXARGS];
    token_t type;
    wordexp_t exp = {0, 0, 0};
    size_t existing_strs;
    int argc = 0;
    bool foreground = 1;
    bool doing_pipe = false;
    int input_fd = STDIN_FILENO;
    int output_fd = STDOUT_FILENO;
    int pipe_fd[2];
    for(;;){
        type = get_token(input, &tokens[argc]);
        switch (type){
            case RAW:
                ++argc;
                break;
            case NORMAL:
                existing_strs = exp.we_wordc;
                if(wordexp(tokens[argc], &exp, WRDE_APPEND)){
                    printf("word expansion failed\n");
                    return;
                }
                for(size_t i = existing_strs; i < exp.we_wordc; ++i){
                    tokens[argc++] = exp.we_wordv[i];
                }
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
                foreground = false;
            case NULLBYTE:
            case SEMICOLON:
                if(argc == 0){
                    return;
                }

                char *aliased_command = get_alias(tokens[0]);
                if(aliased_command){
                    tokens[0] = aliased_command;
                }

                if(strcmp(tokens[0], "ls") == 0){
                    tokens[argc++] = "--color=auto";
                }   
                if(doing_pipe){
                    foreground = true;
                }
                tokens[argc] = NULL;
                fflush(stdout);
                run_program(argc, tokens, foreground, input_fd, output_fd, doing_pipe);
                argc = 0;
                wordfree(&exp);
                exp.we_wordc = 0;
                foreground = true;
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
                    //foreground = false;
                    doing_pipe = false;
                }
                if(type == NULLBYTE){
                    return;
                }
                break;
            default:
                print_error("No matching token type\n");
        }
    }
}