#include "io.h"
#include "command_history.h"
#include <stdlib.h>

void quit(int code){
    restore_terminal();
    save_command_history();
    ch_free();
    exit(code);
}