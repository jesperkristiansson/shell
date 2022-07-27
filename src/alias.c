#include "alias.h"
#include "string_map.h"

static string_map aliases;

void init_aliases(){
    string_map_init(&aliases);
}

void set_alias(char *al, char *cmd){
    string_map_set(&aliases, al, cmd);
}

bool unset_alias(char *al){
    return string_map_remove(&aliases, al);
}

char *get_alias(char *al){
    return string_map_get(&aliases, al);
}

string_map *get_alias_list(){
    return &aliases;
}

void aliases_destructor(){
    string_map_destroy(&aliases);
}