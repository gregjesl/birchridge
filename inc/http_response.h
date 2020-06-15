#ifndef BIRCHRIDGE_HTTP_RESPONSE_H
#define BIRCHRIDGE_HTTP_RESPONSE_H

#ifdef __cplusplus
extern "C" {
#endif

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
key_value_pair_t http_response_find_header(http_response_t response, const char *key);
void http_response_set_header(http_response_t response, const char *key, const char *value);
void http_response_set_content_type(http_response_t response, const char *type);
void http_response_destroy(http_response_t response);

#ifdef __cplusplus
}
#endif

#endif