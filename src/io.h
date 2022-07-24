#ifndef IO_H
#define IO_H

#include <stdio.h>

void switch_terminal();
void init_terminal();
void get_terminal_size();
int fetch_line(char *str_ptr);
int fetch_line_file(char *str, FILE *fp);

#endif