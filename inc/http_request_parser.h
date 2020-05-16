#ifndef BIRCHRIDGE_HTTP_REQUEST_PARSER_H
#define BIRCHRIDGE_HTTP_REQUEST_PARSER_H

#include <stdlib.h>

typedef struct http_request_parser_struct
{
    char **lines;
    size_t num_lines;
} *http_request_parser;

http_request_parser http_request_parser_init(size_t num_lines);
int http_request_parse(http_request_parser request, const char* data);
void http_request_parser_destroy(http_request_parser request);

#endif