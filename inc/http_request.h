#ifndef BIRCHRIDGE_HTTP_REQUEST_H
#define BIRCHRIDGE_HTTP_REQUEST_H

#include <stdlib.h>
#include "key_value_pair.h"
#include "http_resource.h"

enum http_method_enum
{
    HTTP_METHOD_GET = 0,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_CONNECT,
    HTTP_METHOD_OPTIONS,
    HTTP_METHOD_TRACE,
    HTTP_METHOD_PATCH
};

typedef void(http_body_callback)(char *, size_t, void*);

typedef struct http_request_struct
{
    int major_version;
    int minor_version;
    int method;
    http_resource_t resource;
    key_value_linked_list_t headers;
    size_t content_length;
    void *context;
    http_body_callback *body_callback;
    size_t body_remaining;
} *http_request_t;

http_request_t http_request_init();
http_request_t http_parse_request(char **stream);
void http_request_destroy(http_request_t request);

#endif