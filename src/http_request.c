#include "http_request.h"
#include <string.h>

http_request http_request_init()
{
    http_request result = (http_request)malloc(sizeof(struct http_request_struct));

    // Set the lead bytes read
    result->lead_bytes_read = 0;

    // Set the version
    result->major_version = -1;
    result->minor_version = -1;

    // Set the method
    result->method = HTTP_METHOD_GET - 1;

    // Set the path
    result->path = (char*)malloc(sizeof(char));
    result->path[0] = '\0';

    // Set the headers
    result->headers = NULL;

    // Set the content length
    result->content_length = 0;

    // Set the body length
    result->body_callback = NULL;
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

http_request http_parse_request(char **stream)
{
    char *index = *stream;

    // Initialize the result
    http_request result = http_request_init();

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
        const char *endline = strchr(index, ' ');
        if(endline == NULL) {
            http_request_destroy(result);
            return NULL;
        }
        const unsigned int length = endline - index;
        char *path = (char*)malloc(sizeof(char) * (length + 1));
        memcpy(path, index, length * sizeof(char));
        free(result->path);
        result->path = path;
        index += length + 1;
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
    if(*index != '\n') {
        http_request_destroy(result);
        return NULL;
    }
    index++;

    // Parse ALL the headers!
    while(*index != '\n')
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
            char *endline = strchr(index, '\n');
            const unsigned int length = endline - index;
            value = (char*)malloc(sizeof(char) * (length + 1));
            memcpy(value, index, length * sizeof(char));
            value[length] = '\0';
            index = endline + 1;
        }

        http_header header = (http_header)malloc(sizeof(struct http_header_struct));
        header->key = key;
        header->value = value;
        header->next = NULL;
        if(result->headers == NULL) {
            result->headers = header;
        } else {
            http_header current = result->headers;
            while(current->next != NULL) {
                current = current->next;
            }
            current->next = header;
        }

        // Check for a special header
        if(strcmp(key, "Content-Length\0") == 0) {
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
        }
    }

    // Move the input index
    *stream = index + 1;
    return result;
}

/*
int http_request_map(http_request request, const char *data)
{
    const char *index = data;
    const size_t max_bytes = strlen(data);

    // Check for the lead
    while(request->lead_bytes_read < 5) {

        // Check for end of string
        if(*index == '\0') {
            return index - data;
        }

        const char lead[5] = "HTTP ";
        if(*index != lead[request->lead_bytes_read]) {
            return -1;
        }

        request->lead_bytes_read++;
        index++;
    }

    // Check for end
    if(*index == '\0') return index-data;

    // Check for a major version
    if(request->major_version < 0) {
        switch (*index)
        {
        case '1':
            result->major_version = 1;
            break;
        case '2':
            result->major_version = 2;
            break;
        default:
            return -1;
            break;
        }

        index++;
    }

    // Check for end
    if(*index == '\0') return index-data;

    // Check for minor version
    if(request->minor_version < 0) {
        if(*index == '.') {
            index++;
            if(*index == '\0') return index - data;
        }

        switch (*index)
        {
        case '0':
            request->minor_version = 0;
            break;
        case '1':
            request->minor_version = 1;
            break;
        case '2':
            request->minor_version = 2;
            break;
        default:
            return -1;
            break;
        }

        index++;
    }

    // Check for end
    if(*index == '\0') return index-data;


}
*/

void http_request_destroy(http_request request)
{
    // Free the path
    free(request->path);

    // Free the headers
    {
        http_header current = request->headers;
        while(current != NULL) {
            http_header next = current->next;
            free(current->key);
            free(current->value);
            current = next;
        }
    }
}