#include "command_history.h"
#include <stdlib.h>

void quit(int code){
    save_command_history();
    ch_free();
    exit(code);
}