#include "quit.h"
#include "globals.h"
#include "error.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#define DEF_PROMPT "> "

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define CURSOR_FORWARD(x) printf("\033[%dC", (x))
#define CURSOR_BACKWARD(x) printf("\033[%dD", (x))
#define CLEAR_AFTER_CURSOR "\033[J"
#define SAVE_CURSOR "\033[s"
#define RESTORE_CURSOR "\033[u"

#define CTRL_KEY(k) ((k) & 0x1f)
#define ESCAPE (0x1b)
#define BACKSPACE (0x7f)

typedef enum{
    ARROW_LEFT,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
} arrowKey;

static int cpos = 0;
static int prompt_size = 0;

static struct termios orig_term;

void restore_terminal(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term);
}

void init_terminal(){
    tcgetattr(STDIN_FILENO, &orig_term);
    //atexit(restore_terminal);

    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);

    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
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
    prompt_size = strlen(to_print) + sizeof(DEF_PROMPT) - 1;
}

static bool handle_arrow_key(arrowKey key, int right_lim){
    switch (key){
        case ARROW_LEFT:
            if(cpos > prompt_size){
                CURSOR_BACKWARD(1);
                --cpos;
                return true;
            }
            break;
        case ARROW_RIGHT:
            if(cpos < right_lim){
                CURSOR_FORWARD(1);
                ++cpos;
                return true;
            }
            break;
        case ARROW_DOWN:
        case ARROW_UP:
            break;
    }
    return false;
}

int fetch_line(char *str_ptr){  //currently responsible both fetching text AND handling special characters (CTRL+D etc.)
    int c;
    print_prompt();
    cpos = prompt_size;
    size_t input_size = prompt_size;
    str_pos = 0;
    token_pos = 0;
    while((c = getchar())){
        if(c == EOF){
            putchar('\n');
            return EOF;
        }
        switch (c){
            case CTRL_KEY('d'):
                quit();
                break;
            case ESCAPE:
                {
                    char buf[3];
                    buf[0] = getchar();
                    if(buf[0] == '['){
                        buf[1] = getchar();
                        if(buf[1] == 'D'){
                            handle_arrow_key(ARROW_LEFT, input_size);
                        } else if(buf[1] == 'C'){
                            handle_arrow_key(ARROW_RIGHT, input_size);
                        }
                    }
                }
                break;
            case BACKSPACE:
                if(handle_arrow_key(ARROW_LEFT, input_size)){
                    int rel_pos = cpos-prompt_size;
                    memmove(&str_ptr[rel_pos], &str_ptr[rel_pos+1], input_size-rel_pos);
                    --input_size;
                    str_ptr[input_size-prompt_size+1] = '\0';
                    printf(CLEAR_AFTER_CURSOR SAVE_CURSOR "%s" RESTORE_CURSOR, &str_ptr[rel_pos]);
                }
                break;
            default:
                if(!iscntrl(c) || c == '\n'){
                    if(input_size < MAXBUF-1){
                        if(c == '\n'){
                            str_ptr[input_size-prompt_size] = '\n';
                        } else{
                            int rel_pos = cpos-prompt_size;
                            memmove(&str_ptr[rel_pos+1], &str_ptr[rel_pos], MAX(input_size-rel_pos, 0));    //size +1?
                            str_ptr[rel_pos] = (char) c;
                            str_ptr[input_size-prompt_size+1] = '\0';
                            //printf(CLEAR_AFTER_CURSOR SAVE_CURSOR "%s" RESTORE_CURSOR, &str_ptr[rel_pos]);
                        }
                        ++input_size;
                    }
                    putchar(c);
                    ++cpos;
                }
                break;
        }
        if(c == '\n'){
            break;
        }
    }
    if(input_size >= MAXBUF-1){
        print_error("Too many tokens\n");
        return fetch_line(str_ptr);
    }
    str_ptr[input_size-prompt_size] = '\0';
    return input_size-prompt_size;
}