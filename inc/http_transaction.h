#ifndef BIRCHRIDGE_HTTP_TRANSACTION_H
#define BIRCHRIDGE_HTTP_TRANSACTION_H

#include "http_request.h"
#include "http_response.h"
#include "socket_session.h"
#include <stdbool.h>
#include <stdlib.h>

typedef struct http_transaction_struct
{
    http_request_t request;
    http_response_t response;
    socket_wrapper_t session;
    bool response_started;
} *http_transaction_t;

typedef void http_transaction_callback(http_transaction_t, void*);

http_transaction_t http_transaction_init(http_request_t request, socket_wrapper_t session);
ssize_t http_transaction_buffer_request_body(http_transaction_t transaction);
ssize_t http_transaction_read_request_body(http_transaction_t transaction, char *data, const size_t max_length);
void http_transaction_start_response(http_transaction_t transaction);
void http_transaction_send_response_body(http_transaction_t transaction, const char *data, const size_t length);
void http_transaction_destroy(http_transaction_t transaction);

#endif