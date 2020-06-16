#include "http_resource.h"
#include <string.h>
#include <stdlib.h>

http_resource_t http_resource_parse(const char *resource)
{
    const char *index = resource;
    http_resource_t result = (http_resource_t)malloc(sizeof(struct http_resource_struct));

    // Get the first line
    const char *endspace = strchr(index, ' ');
    const size_t max_length = (endspace != NULL) ? endspace - resource : strlen(resource);

    // Get query parameters
    result->query_parameters = key_value_linked_list_init();
    const char *query_start = strchr(resource, '?');
    if(query_start != NULL && query_start < resource + max_length) {
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

        const size_t last_length = (endspace != NULL) ? endspace - index : strlen(index);
        key_value_pair_set_value_n(param, index, last_length);
        key_value_linked_list_append(result->query_parameters, param);
    } else {
        query_start = NULL;
    }

    size_t path_length = 0;
    if(query_start != NULL) {
        path_length = query_start - resource;
    } else {
        // No query
        path_length = max_length;
    }

    result->path = (char*)malloc((path_length + 1) * sizeof(char));
    strncpy(result->path, resource, path_length);
    result->path[path_length] = '\0';
    return result;
}

bool http_path_filter(const char *path, const char *filter)
{
    const char *path_index = path;
    const char *filter_index = filter;

    // Iterate through all levels
    while(strlen(path_index) > 0 && strlen(filter_index) > 0)
    {
        // Strip the leading slash
        if(*path_index != '/' || *filter_index != '/') return false;
        path_index++;
        filter_index++;

        // Check for a wildcard
        if(*filter_index == '#') return true;
        else if(*filter_index == '*') {
            filter_index++;
            const char *next_path = strchr(path_index, '/');
            if(next_path == NULL) {
                return strlen(filter_index) > 0 ? false : true;
            }
            path_index = next_path;
            continue;
        } else {
            // Get the lengths
            const char *next_path = strchr(path_index, '/');
            const size_t path_length = next_path != NULL ? next_path - path_index : strlen(path_index);
            const char *next_filter = strchr(filter_index, '/');
            const size_t filter_length = next_filter != NULL ? next_filter - filter_index : strlen(filter_index);
            if(path_length != filter_length || strncmp(path_index, filter_index, path_length) != 0) return false;
            if(next_path == NULL) {
                return next_filter == NULL;
            } else {
                path_index = next_path;
            }
            if(filter_index == NULL) return false;
            filter_index = next_filter;
        }
    }
    return true;
}

void http_resource_destroy(http_resource_t resource)
{
    key_value_linked_list_destroy(resource->query_parameters);
}