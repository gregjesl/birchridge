#ifndef BIRCHRIDGE_KEY_VALUE_PAIR_H
#define BIRCHRIDGE_KEY_VALUE_PAIR_H

#include <stdlib.h>

typedef struct key_value_pair_struct
{
    char *key;
    char *value;
    struct key_value_pair_struct *next;
} *key_value_pair_t;

key_value_pair_t key_value_pair_init();
void key_value_pair_set_key(key_value_pair_t header, const char *key);
void key_value_pair_set_key_n(key_value_pair_t header, const char *key, size_t length);
void key_value_pair_set_value(key_value_pair_t header, const char *value);
void key_value_pair_set_value_n(key_value_pair_t header, const char *value, size_t length);
key_value_pair_t key_value_pair_destroy(key_value_pair_t header);

typedef key_value_pair_t* key_value_linked_list_t;
key_value_linked_list_t key_value_linked_list_init();
void key_value_linked_list_append(key_value_linked_list_t list, key_value_pair_t addition);
size_t key_value_linked_list_count(const key_value_linked_list_t first);
void key_value_linked_list_destroy(key_value_linked_list_t list);

#endif