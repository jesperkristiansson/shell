#include "io.h"
#include <stdlib.h>

void quit(int code){
    restore_terminal();
    exit(code);
}