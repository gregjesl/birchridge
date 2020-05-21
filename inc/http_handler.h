#ifndef BIRCHRIDGE_HTTP_HANDLER_H
#define BIRCHRIDGE_HTTP_HANDLER_H

#include "http_transaction.h"

typedef struct http_handler_struct
{
    char *filter;
    http_transaction_callback *callback;
    struct http_handler_struct *next;
} *http_handler_t;

http_handler_t http_handler_init(const char *filter, http_transaction_callback *callback);
void http_handler_add(http_handler_t *first, http_handler_t addition);
void http_handler_remove(http_handler_t *first, http_handler_t removal);
bool http_handler_execute(http_handler_t first, http_transaction_t transaction);
http_handler_t http_handler_destroy(http_handler_t handler);

#endif