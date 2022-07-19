#include "command_history.h"
#include "globals.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PATH_FROM_HOME "/.shell_history"
#define HISTORY_MAX_SIZE (500)
#define MOVE_CIRCULAR(val, steps) (val += (steps) % HISTORY_MAX_SIZE)

static char *history[HISTORY_MAX_SIZE];
static int history_index = 0;
static int history_size = 0;
static int history_start = 0;

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
    history_index = 0;
    int line_pos = 0;
    while((c = getc(fp)) != EOF && history_index < HISTORY_MAX_SIZE){
        if(c == '\n'){
            history[history_index++][line_pos] = '\0';
            line_pos = 0;
        } else if(line_pos < MAXBUF-1){ //leaves one char free for a terminating null-byte
            history[history_index][line_pos++] = (char) c;
        }
    }
    history_size = history_index;
    --history_index;    //when reading the file, and extra line is added which should not be included as a command

    fclose(fp);
}

char *previous_command(){
    if(history_index > history_start){
        MOVE_CIRCULAR(history_index, -1);
    }
    return history[history_index];
}

char *next_command(){
    if(history_index < history_size-1){
        MOVE_CIRCULAR(history_index, 1);
        return history[history_index];
    } else{
        return "";
    }
}

void add_command(char * str){
    if(history_size < HISTORY_MAX_SIZE){
        history_index = history_size;
        strncpy(history[history_size++], str, MAXBUF);
    } else{
    }
}

void save_command_history(){
    FILE *fp = open_history_file();
    for(int i = 0; i < history_size; ++i){
        fputs(history[i], fp);
        putc('\n', fp);
    }
    fclose(fp);
}

void ch_free(){
    for(int i = 0; i < HISTORY_MAX_SIZE; ++i){
        free(history[i]);
    }
}