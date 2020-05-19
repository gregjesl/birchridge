#ifndef BIRCHRIDGE_HTTP_HEADER_H
#define BIRCHRIDGE_HTTP_HEADER_H

#include <stdlib.h>

typedef struct http_header_struct
{
    char *key;
    char *value;
    struct http_header_struct *next;
} *http_header_t;

http_header_t http_header_init();
void http_header_set_key(http_header_t header, const char *key);
void http_header_set_value(http_header_t header, const char *value);
void http_header_append(http_header_t *first, http_header_t addition);
size_t http_header_count(const http_header_t first);
http_header_t http_header_destroy(http_header_t header);
void http_header_destroy_chain(http_header_t first);

#endif