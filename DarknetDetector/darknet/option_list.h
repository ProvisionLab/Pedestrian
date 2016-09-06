#ifndef OPTION_LIST_H
#define OPTION_LIST_H
#include "list.h"

typedef struct{
    char *key;
    char *val;
    int used;
} kvp;


int read_option(char *s, list *options);
void option_insert(list *l, const char *key, char *val);
const char *option_find(list *l, const char *key);
const char *option_find_str(list *l, const char *key,const  char *def);
int option_find_int(list *l, const char *key, int def);
int option_find_int_quiet(list *l, const char *key, int def);
float option_find_float(list *l, const char *key, float def);
float option_find_float_quiet(list *l, const char *key, float def);
void option_unused(list *l);

#endif
