#ifndef COMMAND_HISTORY_H
#define COMMAND_HISTORY_H

void ch_init();
char *previous_command();
char *next_command();
void add_command(char * str);
void save_command_history();
int get_history(char ***ptr);
void ch_free();

#endif