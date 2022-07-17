#ifndef IO_H
#define IO_H

void restore_terminal();
void init_terminal();
void get_terminal_size();
int fetch_line(char *str_ptr);
void parse_line(char *input);

#endif