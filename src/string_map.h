#ifndef STRING_MAP_H
#define STRING_MAP_H

#include <stdbool.h>

typedef struct string_map_node{
    char *key, *val;
    struct string_map_node *next;
} string_map_node;


typedef struct string_map{
    string_map_node *head;
} string_map;

void string_map_init(string_map *map);
void string_map_set(string_map *map, const char *key, const char *val);
char *string_map_get(string_map *map, const char *key);
bool string_map_remove(string_map *map, const char *key);
void string_map_destroy(string_map *map);

#endif