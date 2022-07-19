#ifndef EXECUTE_H
#define EXECUTE_H

#include <stdbool.h>

void cd(int argc, char **argv);
void help();
void print_history();
void set_env_var(int argc, char **argv);
bool check_builtins(int argc, char **argv);
void run_program(int argc, char **argv, bool foreground, int input_fd, int output_fd, bool doing_pipe);
void parse_line(char *input);

#endif