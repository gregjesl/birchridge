#ifndef BIRCHRIDGE_HTTP_RESOURCE_H
#define BIRCHRIDGE_HTTP_RESOURCE_H

#include "key_value_pair.h"
#include "stdbool.h"

typedef struct http_resource_struct
{
    char *path;
    key_value_linked_list_t query_parameters;
} *http_resource_t;

http_resource_t http_resource_parse(const char *resource);
bool http_path_filter(const char *path, const char *filter);
void http_resource_destroy(http_resource_t resource);

#endif