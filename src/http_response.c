#include "http_response.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

http_response_t http_response_init(const int major_version, const int minor_version)
{
    http_response_t result = (http_response_t)malloc(sizeof(struct http_response_struct));
    result->major_version = major_version;
    result->minor_version = minor_version;
    result->status_code = -1;
    result->headers = key_value_linked_list_init();
    return result;
}

key_value_pair_t http_response_find_header(http_response_t response, const char *key)
{
    if(*response->headers == NULL) return NULL;
    key_value_pair_t current = *response->headers;
    key_value_pair_t next = NULL;
    while(current != NULL) {
        if(strcmp(key, current->key) == 0) {
            return current;
        }
        next = current->next;
        current = next;
    }
    return NULL;
}

void http_response_set_header(http_response_t response, const char *key, const char *value)
{
    key_value_pair_t existing = http_response_find_header(response, key);
    if(existing == NULL) {
        existing = key_value_pair_init();
        key_value_pair_set_key(existing, key);
        key_value_linked_list_append(response->headers, existing);
    }
    key_value_pair_set_value(existing, value);
}

void http_response_set_content_type(http_response_t response, const char *type)
{
    http_response_set_header(response, "Content-Type", type);
}

void http_response_destroy(http_response_t response)
{
    key_value_linked_list_destroy(response->headers);
    free(response);
}