#include "string_map.h"
#include <stdlib.h>
#include <string.h>

static string_map_node *create_node(const char *key, const char *val){
    string_map_node *node = malloc(sizeof(string_map_node));

    node->key = malloc(strlen(key)+1);
    strcpy(node->key, key);

    node->val = malloc(strlen(val)+1);
    strcpy(node->val, val);

    node->next = NULL;
    return node;
}

void string_map_init(string_map *map){
    map->head = NULL;
}

void string_map_set(string_map *map, const char *key, const char *val){
    string_map_node *current = map->head;
    while(current != NULL){         //check if key already has a mapping and if so, replace it
        if(strcmp(current->key, key) == 0){
            current->val = realloc(current->val, strlen(val)+1);
            strcpy(current->val, val);
            return;
        }
        current = current->next;
    }

    string_map_node *node = create_node(key, val);      //if it is a new key, insert a new node at the start of map
    node->next = map->head;
    map->head = node;
}

char *string_map_get(string_map *map, const char *key){
    string_map_node *current = map->head;
    while(current != NULL){         //check if key already has a mapping and if so, replace it
        if(strcmp(current->key, key) == 0){
            return current->val;
        }
        current = current->next;
    }
    return NULL;
}

bool string_map_remove(string_map *map, const char *key){
    string_map_node **last_ptr = &map->head;
    string_map_node *current = map->head;
    
    while(current != NULL){
        if(strcmp(current->key, key) == 0){
            *last_ptr = current->next;
            free(current->key);
            free(current->val);
            free(current);
            return true;
        }
        last_ptr = &current->next;
        current = current->next;
    }
    return false;
}

void string_map_destroy(string_map *map){
    string_map_node *current = map->head;
    while(current != NULL){
        string_map_node *next = current->next;
        free(current->key);
        free(current->val);
        free(current);
        current = next;
    }
}