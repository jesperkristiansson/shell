#ifndef ERROR_H
#define ERROR_H
#include <errno.h>
#include <string.h>

#define PRINT_ERROR printf("%s\n", strerror(errno))

void print_error(const char *fmt, ...);

#endif