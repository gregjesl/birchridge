#ifndef BIRCHRIDGE_HTTP_RESPONSE_H
#define BIRCHRIDGE_HTTP_RESPONSE_H

#include <stdlib.h>
#include "http_header.h"

typedef struct http_response_struct
{
    int major_version;
    int minor_version;
    int status_code;
    http_header_t headers;
    size_t content_length;
    void *context;
    size_t body_remaining;
} *http_response_t;

http_response_t http_response_init(const int major_version, const int minor_version);
void http_response_set_body_length(http_response_t response, size_t length);
void http_response_destroy(http_response_t response);

#endif