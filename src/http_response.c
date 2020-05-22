#include "http_response.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

const char *content_length_key = "Content-Length\0";

http_response_t http_response_init(const int major_version, const int minor_version)
{
    http_response_t result = (http_response_t)malloc(sizeof(struct http_response_struct));
    result->major_version = major_version;
    result->minor_version = minor_version;
    result->status_code = -1;
    result->headers = key_value_linked_list_init();
    result->body_remaining = 0;
    return result;
}

void http_response_set_body_length(http_response_t response, size_t length)
{
    assert(response->headers != NULL);
    key_value_pair_t current = *response->headers;
    key_value_pair_t next = NULL;
    if(current == NULL) {
        current = key_value_pair_init();
        key_value_linked_list_append(response->headers, current);
        key_value_pair_set_key(current, content_length_key);
    } else {
        while(strcmp(current->key, content_length_key) != 0) {
            next = current->next;
            current = next;
            if(current == NULL) {
                current = key_value_pair_init();
                key_value_linked_list_append(response->headers, current);
                key_value_pair_set_key(current, content_length_key);
                break;
            }
        }
    }

    // Determine the number of digits
    size_t digits = 1;
    size_t level = length / 10;
    while (level > 0) {
        digits++;
        level /= 10;
    }
    char *value = (char*)malloc((digits + 1) * sizeof(char));
    sprintf(value, "%lu", length);
    key_value_pair_set_value(current, value);
    free(value);
    response->body_remaining = length;
}

void http_response_destroy(http_response_t response)
{
    key_value_linked_list_destroy(response->headers);
    free(response);
}