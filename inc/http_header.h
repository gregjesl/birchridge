#ifndef BIRCHRIDGE_HTTP_HEADER_H
#define BIRCHRIDGE_HTTP_HEADER_H

typedef struct http_header_struct
{
    char *key;
    char *value;
    struct http_header_struct *next;
} *http_header;

http_header http_header_init();
void http_header_set_key(http_header header, const char *key);
void http_header_set_value(http_header header, const char *value);
void http_header_append(http_header *first, http_header addition);
http_header http_header_destroy(http_header header);
void http_header_destroy_chain(http_header first);

#endif