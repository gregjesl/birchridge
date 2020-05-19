#include "http_header.h"
#include <assert.h>
#include <string.h>

http_header http_header_init()
{
    http_header result = (http_header)malloc(sizeof(struct http_header_struct));
    result->key = NULL;
    result->value = NULL;
    result->next = NULL;
    return result;
}

void http_header_string_copy(char **output, const char *input)
{
    assert(output != NULL);
    assert(input != NULL);

    if(*output != NULL) 
        free(*output);
    const size_t length = strlen(input) + 1;
    *output = (char*)malloc(length * sizeof(char));
    memcpy(*output, input, length);
}

void http_header_set_key(http_header header, const char *key)
{
    http_header_string_copy(&header->key, key);
}

void http_header_set_value(http_header header, const char *value)
{
    http_header_string_copy(&header->value, value);
}

void http_header_append(http_header *first, http_header addition)
{
    assert(first != NULL);

    if(*first == NULL) {
        *first = addition;
        return;
    }

    http_header current = *first;
    http_header next = NULL;
    while(current->next != NULL) {
        next = current->next;
        current = next;
    }
    current->next = addition;
}

size_t http_header_count(http_header first)
{
    http_header current = first;
    http_header next = NULL;
    size_t result = 0;
    while(current != NULL) {
        next = current->next;
        current = next;
        result++;
    }
    return result;
}

http_header http_header_destroy(http_header header)
{
    if(header == NULL) return NULL;

    http_header result = header->next;
    free(header->key);
    free(header->value);
    free(header);
    return result;
}

void http_header_destroy_chain(http_header first)
{
    http_header current = first;
    http_header next = NULL;
    while(current != NULL) {
        next = http_header_destroy(current);
        current = next;
    }
}