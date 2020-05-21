#include "http_resource.h"
#include <string.h>
#include <stdlib.h>

http_resource_t http_resource_parse(const char *resource)
{
    const char *index = resource;
    http_resource_t result = (http_resource_t)malloc(sizeof(struct http_resource_struct));

    // Get query parameters
    result->query_parameters = key_value_linked_list_init();
    const char *query_start = strchr(resource, '?');
    if(query_start != NULL) {
        // Move past the question mark
        index = query_start + 1;

        // Get the key
        const char *value_start = strchr(index, '=');
        const char *delim = strchr(index, '&');
        while(delim != NULL) {
            if(value_start == NULL) {
                http_resource_destroy(result);
                return NULL;
            }
            key_value_pair_t param = key_value_pair_init();
            key_value_pair_set_key_n(param, index, value_start - index);
            index = value_start + 1;
            key_value_pair_set_value_n(param, index, delim - index);
            key_value_linked_list_append(result->query_parameters, param);
            index = delim + 1;
            value_start = strchr(index, '=');
            delim = strchr(index, '&');
        }
        key_value_pair_t param = key_value_pair_init();
        key_value_pair_set_key_n(param, index, value_start - index);
        index = value_start + 1;

        const char *endspace = strchr(index, ' ');
        const size_t last_length = (endspace != NULL) ? endspace - index : strlen(index);
        key_value_pair_set_value_n(param, index, last_length);
        key_value_linked_list_append(result->query_parameters, param);
    }

    const size_t path_length = query_start != NULL ? query_start - resource : strlen(resource);
    result->path = (char*)malloc((path_length + 1) * sizeof(char));
    strncpy(result->path, resource, path_length);
    return result;
}

void http_resource_destroy(http_resource_t resource)
{
    key_value_linked_list_destroy(resource->query_parameters);
}