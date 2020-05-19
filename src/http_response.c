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
    result->status_code = 204;
    result->headers = http_header_init();
    http_header_set_key(result->headers, content_length_key);
    http_header_set_value(result->headers, "0\0");
    result->body_remaining = 0;
    return result;
}

void http_response_set_body_length(http_response_t response, size_t length)
{
    assert(response->headers != NULL);
    http_header_t current = response->headers;
    http_header_t next = NULL;
    while(strcmp(current->key, content_length_key) != 0) {
        next = current->next;
        current = next;
        assert(current != NULL);
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
    http_header_set_value(current, value);
    free(value);
}

void http_response_destroy(http_response_t response)
{
    http_header_destroy_chain(response->headers);
    free(response);
}