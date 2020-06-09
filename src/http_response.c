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

void http_response_destroy(http_response_t response)
{
    key_value_linked_list_destroy(response->headers);
    free(response);
}