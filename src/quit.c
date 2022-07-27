#include "quit.h"
#include "command_history.h"
#include "alias.h"
#include <stdlib.h>

void quit(int code){
    save_command_history();
    ch_free();
    aliases_destructor();
    exit(code);
}