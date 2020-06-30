#include "http_request.h"
#include <string.h>
#include <assert.h>

#define IS_NEW_LINE(a) (strncmp(a, "\r\n", 2) == 0)

http_request_t http_request_init()
{
    http_request_t result = (http_request_t)malloc(sizeof(struct http_request_struct));

    // Set the version
    result->major_version = -1;
    result->minor_version = -1;

    // Set the method
    result->method = HTTP_METHOD_GET - 1;

    // Set the resource
    result->resource = NULL;

    // Set the headers
    result->headers = key_value_linked_list_init();

    // Set the content length
    result->content_length = 0;

    // Set the keep alive
    result->keep_alive = false;

    // Set the body length
    result->body_buffer = NULL;
    result->body_buffer_length = 0;
    result->body_remaining = 0;

    return result;
}

size_t scan_int(char value)
{
    switch (value)
    {
    case '0':
        return 0;
        break;
    case '1':
        return 1;
        break;
    case '2':
        return 2;
        break;
    case '3':
        return 3;
        break;
    case '4':
        return 4;
        break;
    case '5':
        return 5;
        break;
    case '6':
        return 6;
        break;
    case '7':
        return 7;
        break;
    case '8':
        return 8;
        break;
    case '9':
        return 9;
        break;
    default:
        return 10;
        break;
    }
}

http_request_t http_parse_request(char **stream)
{
    char *index = *stream;

    // Initialize the result
    http_request_t result = http_request_init();

    // Read the method
    {
        const char *space = strchr(index, ' ');
        if(space == NULL) {
            http_request_destroy(result);
            return NULL;
        }
        const unsigned int length = space - index;
        char method[8];
        memcpy(method, index, length * sizeof(char));
        method[length] = '\0';

        const char* methods[9] = {
            "GET\0",
            "HEAD\0",
            "POST\0",
            "PUT\0",
            "DELETE\0",
            "CONNECT\0",
            "OPTIONS\0",
            "TRACE\0",
            "PATCH\0"
        };

        for(size_t i = 0; i < 9; i++) {
            if(strcmp(method, methods[i]) == 0) {
                result->method = i;
                break;
            }
        }
        if(result->method < 0) {
            http_request_destroy(result);
            return NULL;
        }
        index += length + 1;
    }

    // Read the path
    {
        char *endline = strchr(index, ' ');
        if(endline == NULL) {
            http_request_destroy(result);
            return NULL;
        }
        result->resource = http_resource_parse(index);
        assert(strchr(result->resource->path, ' ') == NULL);
        assert(strchr(result->resource->path, '?') == NULL);
        index = endline + 1;
    }

    // Verify the beginning
    {
        const char beginning[5] = "HTTP/";
        for(size_t i = 0; i < 5; i++) {
            if(index[i] != beginning[i]) {
                http_request_destroy(result);
                return NULL;
            }
        }
        index += 5;
    }

    // Read the major version
    switch (*index)
    {
    case '1':
        result->major_version = 1;
        break;
    case '2':
        result->major_version = 2;
        break;
    default:
        return NULL;
        break;
    }
    index++;

    // Skip the dot
    if(*index != '.') return NULL;
    index++;

    // Read the minor version
    switch (*index)
    {
    case '0':
        result->minor_version = 0;
        break;
    case '1':
        result->minor_version = 1;
        break;
    case '2':
        result->minor_version = 2;
        break;
    default:
        return NULL;
        break;
    }
    index++;

    // Verify the newline
    if(!IS_NEW_LINE(index)) {
        http_request_destroy(result);
        return NULL;
    }
    index += 2;

    // Parse ALL the headers!
    while(!IS_NEW_LINE(index))
    {
        char *key = NULL;
        char *value = NULL;
        char *colon = strchr(index, ':');
        if(colon == NULL) {
            http_request_destroy(result);
            return NULL;
        }

        // Read the key
        {
            const unsigned int length = colon - index;
            key = (char*)malloc(length + 1);
            memcpy(key, index, length * sizeof(char));
            key[length] = '\0';
            index = colon + 1;
        }

        // Verify the space
        if(*index != ' ') {
            http_request_destroy(result);
            return NULL;
        }
        index++;

        // Read the value
        {
            char *endline = strstr(index, "\r\n");
            const unsigned int length = endline - index;
            value = (char*)malloc(sizeof(char) * (length + 1));
            memcpy(value, index, length * sizeof(char));
            value[length] = '\0';
            index = endline + 2;
        }

        key_value_pair_t header = key_value_pair_init();
        key_value_pair_set_key(header, key);
        key_value_pair_set_value(header, value);
        key_value_linked_list_append(result->headers, header);

        // Check for a special header
        if(strcmp(key, "Content-Length") == 0) {
            const char digits = strlen(value);
            result->content_length = scan_int(*value);
            if(result->content_length > 9) {
                http_request_destroy(result);
                return NULL;
            }
            for(size_t i = 1; i < digits; i++) {
                result->content_length *= 10;
                const size_t next = scan_int(value[i]);
                result->content_length += next;
            }
            result->body_remaining = result->content_length;
        } else if(strcmp(key, "Connection") == 0) {
            if(strcmp(value, "Keep-Alive") == 0) {
                result->keep_alive = true;
            }
        }
    }

    // Move the buffer past the last newline
    *stream = index + 2;
    return result;
}

const char * http_request_find_header(http_request_t request, const char *key)
{
    key_value_pair_t pair = key_value_find(request->headers, key);
    return pair != NULL ? pair->key : NULL;
}

const char * http_request_find_query(http_request_t request, const char *key)
{
    key_value_pair_t pair = key_value_find(request->resource->query_parameters, key);
    return pair != NULL ? pair->value : NULL;
}

void http_request_destroy(http_request_t request)
{
    // Free the path
    if(request->resource != NULL)
        http_resource_destroy(request->resource);

    // Free the headers
    key_value_linked_list_destroy(request->headers);
}