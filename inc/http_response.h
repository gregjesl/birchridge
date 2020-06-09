#ifndef BIRCHRIDGE_HTTP_RESPONSE_H
#define BIRCHRIDGE_HTTP_RESPONSE_H

#include <stdlib.h>
#include "key_value_pair.h"

typedef struct http_response_struct
{
    int major_version;
    int minor_version;
    int status_code;
    key_value_linked_list_t headers;
} *http_response_t;

http_response_t http_response_init(const int major_version, const int minor_version);
void http_response_destroy(http_response_t response);

#endif