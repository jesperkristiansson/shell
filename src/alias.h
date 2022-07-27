#ifndef ALIAS_H
#define ALIAS_H

#include <stdbool.h>
#include "string_map.h"

void init_aliases();
void set_alias(char *al, char *cmd);
bool unset_alias(char *al);
char *get_alias(char *al);
string_map *get_alias_list();
void aliases_destructor();

#endif