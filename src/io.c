#include "io.h"
#include "quit.h"
#include "globals.h"
#include "error.h"
#include "command_history.h"
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
#define INPUT_POS (cpos - prompt_size)
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
    ARROW_LEFT = 'D',
    ARROW_RIGHT = 'C',
    ARROW_UP = 'A',
    ARROW_DOWN = 'B',
} arrowKey;

static char *input;
static int cpos = 0;
static int prompt_size = 0;
static int input_size = 0;

static int cols_size = 0;

static struct termios orig_term;

void reset_terminal(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term);
}

void init_terminal(){
    tcgetattr(STDIN_FILENO, &orig_term);

    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);

    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    get_terminal_size();
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

static void remove_input();

static bool handle_arrow_key(arrowKey key){ //can't move down to next row when at the end of line
    int row, col;
    cursor_col_position(&row, &col);
    char *cmd;
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
            remove_input();
            cmd = next_command();
            strncpy(input, cmd, MAXBUF);
            printf("%s", input);
            input_size = strlen(input);
            cpos += input_size;
            break;
        case ARROW_UP:
            remove_input();
            cmd = previous_command();
            strncpy(input, cmd, MAXBUF);
            printf("%s", input);
            input_size = strlen(input);
            cpos += input_size;
            break;
    }
    return false;
}

static bool delete_next_char(){
    if(INPUT_POS >= input_size){
        return false;
    }
    memmove(&input[INPUT_POS], &input[INPUT_POS+1], input_size-INPUT_POS);
    --input_size;
    input[input_size] = '\0';
    printf(CLEAR_AFTER_CURSOR SAVE_CURSOR "%s" RESTORE_CURSOR, &input[INPUT_POS]);
    return true;
}

static bool delete_char(){
    if(handle_arrow_key(ARROW_LEFT)){
        return delete_next_char(input);
    }
    return false;
}

static void remove_input(){
    while(handle_arrow_key(ARROW_LEFT));
    printf(CLEAR_AFTER_CURSOR);
    input[0] = '\0';
    input_size = 0;
}

static void handle_escape_sequence(){
    char buf[5];
    buf[0] = getchar(); 
    if(buf[0] == '['){
        buf[1] = getchar();
        switch(buf[1]){
            case ARROW_UP:
            case ARROW_DOWN:
            case ARROW_RIGHT:
            case ARROW_LEFT:
                handle_arrow_key(buf[1]);
                break;
            case '1':
                buf[2] = getchar();
                buf[3] = getchar();
                buf[4] = getchar();
                if(buf[2] == ';' && buf[3] == '5'){
                    if(buf[4] == ARROW_LEFT){
                        while(handle_arrow_key(buf[4]) && input[INPUT_POS-1] != ' ');
                    } else if(buf[4] == ARROW_RIGHT){
                        while(handle_arrow_key(buf[4]) && input[INPUT_POS] != ' ');
                    }
                }
                break;
            case '3':
                buf[2] = getchar();
                if(buf[2] == '~'){
                    delete_next_char(input);
                }
                break;
        }
    }
}

void handle_normal_char(char c){
    putchar(c);
    if(input_size < MAXBUF-1){
        if(INPUT_POS < input_size){
            memmove(&input[INPUT_POS+1], &input[INPUT_POS], MAX(input_size-INPUT_POS, 0));    //size +1?
            input[input_size+1] = '\0';
            printf(CLEAR_AFTER_CURSOR SAVE_CURSOR "%s" RESTORE_CURSOR, &input[INPUT_POS+1]);
        }
        input[INPUT_POS] = (char) c;
        ++input_size;
    }
    ++cpos;
}

int fetch_line(char *str){  //currently responsible both fetching text and handling special characters (CTRL+D etc.)
    init_terminal();
    input = str;
    int c;
    print_prompt();
    cpos = prompt_size;
    input_size = 0;
    while((c = getchar()) != '\n'){
        if(c == EOF){
            putchar('\n');
            reset_terminal();
            return EOF;
        }
        switch (c){
            case CTRL_KEY('d'):
                if(input_size == 0){
                    putchar('\n');
                    reset_terminal();
                    return EOF;
                } else{
                    delete_next_char();
                }
                break;
            case CTRL_KEY('c'): //remove current input
                remove_input();
                break;
            case CTRL_KEY('k'):
                printf(CLEAR_AFTER_CURSOR);
                input[INPUT_POS] = '\0';
                input_size = INPUT_POS;
                break;
            case CTRL_KEY('w'): //remove one word of input
                while(delete_char() && INPUT_POS > 0 && input[INPUT_POS-1] != ' ');
                break;
            case CTRL_KEY('u'):
                while(delete_char());
                break;
            case ESCAPE:
                handle_escape_sequence();
                break;
            case BACKSPACE:
                delete_char();
                break;
            default:
                if(!iscntrl(c)){
                    handle_normal_char(c);
                } else{
                    printf("received character %d\n", c);
                }
                break;
        }
    }
    putchar('\n');
    reset_terminal();
    if(input_size >= MAXBUF-1){
        print_error("Too many tokens\n");
        return fetch_line(input);
    }
    input[input_size] = '\0';
    add_command(input);
    return input_size;
}

int fetch_line_file(char *str, FILE *fp){
    int i = 0;
    int c;
    while((c = getc(fp)) != '\n'){
        if(c == EOF){
            return EOF;
        } else if(i < MAXBUF-1){
            str[i++] = (char) c;
        }
    }
    if(i >= MAXBUF-1){
        print_error("Too many tokens\n");
        return EOF;
    }
    str[i++] = '\0';
    return i;
}