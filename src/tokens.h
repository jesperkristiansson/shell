#ifndef TOKENS_H
#define TOKENS_H

#include <stdbool.h>

typedef enum{
    INPUT,
    OUTPUT,
    PIPE,
    NEWLINE,
    SEMICOLON,
    AMPERSAND,
    NORMAL,
    RAW
} token_t;

bool end_of_token(char c);
token_t get_token(char *str, char **token_ptr);

#endif