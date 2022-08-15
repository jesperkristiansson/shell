#include "config.h"
#include "globals.h"
#include "io.h"
#include "execute.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define CONFIG_PATH_FROM_HOME "/.shell_config"

void run_config(){                  //check if config file exists
    char path[MAXBUF];
    strncpy(path, getenv("HOME"), MAXBUF-strlen(CONFIG_PATH_FROM_HOME));
    strcat(path, CONFIG_PATH_FROM_HOME);

    if(access(path, R_OK) != 0){
        return;
    }
    FILE *config_file = fopen(path, "r");

    char buf[MAXBUF];
    while(fetch_line_file(buf, config_file) != EOF){
        parse_line(buf);
    }

    fclose(config_file);
}