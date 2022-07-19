#include "command_history.h"
#include "globals.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define PATH_FROM_HOME "/.shell_history"
#define HISTORY_MAX_SIZE (500)

static char *history[HISTORY_MAX_SIZE];
static int history_index = 0;   //index of current command selected
static int history_size = 0;

static FILE *open_history_file(){
    char path[MAXBUF];
    strncpy(path, getenv("HOME"), MAXBUF-strlen(PATH_FROM_HOME));
    strcat(path, PATH_FROM_HOME);
    int fd = open(path, O_RDWR | O_CREAT, 0666);
    return fdopen(fd, "r+");
}

void ch_init(){ //read from file to history array
    FILE *fp = open_history_file();

    for(int i = 0; i < HISTORY_MAX_SIZE; ++i){
        history[i] = malloc(sizeof(char)*MAXBUF);
    }

    int c;
    char buf[MAXBUF];
    int line_pos = 0;
    while((c = getc(fp)) != EOF){
        if(c == '\n'){
            buf[line_pos] = '\0';
            add_command(buf);
            line_pos = 0;
        } else if(line_pos < MAXBUF-1){ //leaves one char free for a terminating null-byte
            buf[line_pos++] = (char) c;
        }
    }

    fclose(fp);
}

char *previous_command(){
    if(history_index > 0){
        --history_index;
    }
    return history[history_index];
}

char *next_command(){
    history_index = MIN(history_index+1, history_size);
    if(history_index < history_size){
        return history[history_index];
    } else{
        return "";
    }
}

void add_command(char *cmd){    //can likely be optimized by using a circular array instead of moving all elements
    if(history_size < HISTORY_MAX_SIZE){
        strncpy(history[history_size], cmd, MAXBUF);
        history_index = ++history_size;
    } else{
        for(int i = 1; i < HISTORY_MAX_SIZE; i++){      //shift all strings in the history-array one step to make room
           memcpy(history[i-1], history[i], MAXBUF);
        }
        strncpy(history[history_size-1], cmd, MAXBUF);
        history_index = history_size;
    }
}

void save_command_history(){
    FILE *fp = open_history_file();
    fp = freopen(NULL, "w", fp);    //clears the content of the history-file
    for(int i = 0; i < history_size; ++i){
        fputs(history[i], fp);
        putc('\n', fp);
    }
    fclose(fp);
}

int get_history(char ***ptr){
    *ptr = history;
    return history_size;
}

void ch_free(){
    for(int i = 0; i < HISTORY_MAX_SIZE; ++i){
        free(history[i]);
    }
}