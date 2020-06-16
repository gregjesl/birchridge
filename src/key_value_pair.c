#include "key_value_pair.h"
#include <assert.h>
#include <string.h>

key_value_pair_t key_value_pair_init()
{
    key_value_pair_t result = (key_value_pair_t)malloc(sizeof(struct key_value_pair_struct));
    result->key = NULL;
    result->value = NULL;
    result->next = NULL;
    return result;
}

void _string_copy(char **output, const char *input, size_t length)
{
    assert(output != NULL);
    assert(input != NULL);

    if(*output != NULL) 
        free(*output);
    *output = (char*)malloc((length + 1) * sizeof(char));
    strncpy(*output, input, length);
    *output[length] = '\0';
}

void key_value_pair_set_key(key_value_pair_t header, const char *key)
{
    _string_copy(&header->key, key, strlen(key));
}

void key_value_pair_set_key_n(key_value_pair_t header, const char *key, size_t length)
{
    _string_copy(&header->key, key, length);
}

void key_value_pair_set_value(key_value_pair_t header, const char *value)
{
    _string_copy(&header->value, value, strlen(value));
}

void key_value_pair_set_value_n(key_value_pair_t header, const char *value, size_t length)
{
    _string_copy(&header->value, value, length);
}

key_value_pair_t key_value_pair_destroy(key_value_pair_t header)
{
    if(header == NULL) return NULL;
    if(header->key != NULL) free(header->key);
    if(header->value != NULL) free(header->value);
    key_value_pair_t result = header->next;
    free(header);
    return result;
}

key_value_linked_list_t key_value_linked_list_init()
{
    key_value_linked_list_t result = malloc(sizeof(key_value_linked_list_t));
    *result = NULL;
    return result;
}

void key_value_linked_list_append(key_value_linked_list_t list, key_value_pair_t addition)
{
    assert(list != NULL);

    // Check for first entry in list
    if(*list == NULL) {
        *list = addition;
        return;
    }

    key_value_pair_t current = *list;
    key_value_pair_t next = NULL;
    while(current->next != NULL) {
        next = current->next;
        current = next;
    }
    current->next = addition;
}

size_t key_value_linked_list_count(const key_value_linked_list_t list)
{
    key_value_pair_t current = *list;
    key_value_pair_t next = NULL;
    size_t result = 0;
    while(current != NULL) {
        next = current->next;
        current = next;
        result++;
    }
    return result;
}

key_value_pair_t key_value_find(const key_value_linked_list_t first, const char *key)
{
    if(first == NULL || *first == NULL) return NULL;
    key_value_pair_t current = *first;
    key_value_pair_t next = NULL;
    size_t result = 0;
    while(current != NULL) {
        if(strcmp(current->key, key) == 0) {
            return current;
        }
        next = current->next;
        current = next;
    }
    return NULL;
}

void key_value_linked_list_destroy(key_value_linked_list_t list)
{
    key_value_pair_t current = *list;
    key_value_pair_t next = NULL;
    while(current != NULL) {
        next = key_value_pair_destroy(current);
        current = next;
    }
}