#include "io.h"
#include <stdlib.h>

void quit(){
    restore_terminal();
    exit(0);
}