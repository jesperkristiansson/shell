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
#include <sys/ioctl.h>

#define DEF_PROMPT "> "

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define CURSOR_FORWARD(x) printf("\033[%dC", (x))
#define CURSOR_BACKWARD(x) printf("\033[%dD", (x))
#define CURSOR_TO_POS(x, y) printf("\033[%d;%dH", (x), (y))
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
static int input_size = 0;

static int cols_size = 0;

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

void get_terminal_size(){   //add manual calculation of size when ioctl fails
    struct winsize wz;

    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &wz) < 0 || wz.ws_col == 0){
        quit(-1);
    }
    cols_size = wz.ws_col;
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

static int cursor_col_position(int *row, int *col){
    char buf[32];

    printf("\033[6n");
    //fflush(stdout);
    int i;
    for(i = 0; i < 32; ++i){
        buf[i] = getchar();
        if(buf[i] == 'R'){
            break;
        }
    }
    buf[i] = '\0';
    
    if(buf[0] != '\033' || buf[1] != '['){
        return -1;
    }
    if(sscanf(&buf[2], "%d;%d", row, col) != 2){
        return -1;
    }

    return 0;
}

static bool handle_arrow_key(arrowKey key){ //can't move down to next row when at the end of line
    int row, col;
    cursor_col_position(&row, &col);
    switch (key){
        case ARROW_LEFT:
            if(cpos > prompt_size){
                if(col > 1){
                    CURSOR_BACKWARD(1);
                } else{
                    CURSOR_TO_POS(row-1, cols_size);
                }
                --cpos;
                return true;
            }
            break;
        case ARROW_RIGHT:
            if(cpos < prompt_size + input_size){
                if(col < cols_size){
                    CURSOR_FORWARD(1);
                } else{
                    CURSOR_TO_POS(row+1, 0);
                }
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
    input_size = 0;
    str_pos = 0;
    token_pos = 0;
    while((c = getchar())){
        if(c == EOF){
            putchar('\n');
            return EOF;
        }
        if(c == '\n'){
            break;
        }
        switch (c){
            case CTRL_KEY('d'):
                putchar('\n');
                quit(0);
                break;
            case ESCAPE:
                {
                    char buf[3];
                    buf[0] = getchar();
                    if(buf[0] == '['){
                        buf[1] = getchar();
                        if(buf[1] == 'D'){
                            handle_arrow_key(ARROW_LEFT);
                        } else if(buf[1] == 'C'){
                            handle_arrow_key(ARROW_RIGHT);
                        }
                    }
                }
                break;
            case BACKSPACE:
                if(handle_arrow_key(ARROW_LEFT)){
                    int rel_pos = cpos-prompt_size;
                    memmove(&str_ptr[rel_pos], &str_ptr[rel_pos+1], input_size-rel_pos);
                    --input_size;
                    str_ptr[input_size] = '\0';
                    printf(CLEAR_AFTER_CURSOR SAVE_CURSOR "%s" RESTORE_CURSOR, &str_ptr[rel_pos]);
                }
                break;
            default:
                if(!iscntrl(c)){
                    putchar(c);
                    if(input_size < MAXBUF-1){
                        int rel_pos = cpos-prompt_size;
                        if(rel_pos < input_size){
                            memmove(&str_ptr[rel_pos+1], &str_ptr[rel_pos], MAX(input_size-rel_pos, 0));    //size +1?
                            str_ptr[input_size+1] = '\0';
                            printf(CLEAR_AFTER_CURSOR SAVE_CURSOR "%s" RESTORE_CURSOR, &str_ptr[rel_pos+1]);
                        }
                        str_ptr[rel_pos] = (char) c;
                        ++input_size;
                    }
                    ++cpos;
                } else{
                    printf("received character %d\n", c);
                }
                break;
        }
    }
    putchar('\n');
    if(input_size >= MAXBUF-1){
        print_error("Too many tokens\n");
        return fetch_line(str_ptr);
    }
    str_ptr[input_size] = '\0';
    return input_size;
}