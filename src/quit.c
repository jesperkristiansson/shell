#include "io.h"
#include <stdlib.h>

void restore(){
    restore_terminal();
}

void quit(){
    restore();
    exit(0);
}