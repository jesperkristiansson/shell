#include "error.h"
#include <stdio.h>
#include <stdarg.h>

void print_error(const char *fmt, ...){
    va_list ap;

    fprintf(stderr, "error: ");

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}