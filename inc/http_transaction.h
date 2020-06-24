#ifndef BIRCHRIDGE_HTTP_TRANSACTION_H
#define BIRCHRIDGE_HTTP_TRANSACTION_H

#ifdef __cplusplus
extern "C" {
#endif

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
    bool head_sent;
    bool response_complete;
} *http_transaction_t;

typedef void http_transaction_callback(http_transaction_t, void*);

http_transaction_t http_transaction_init(http_request_t request, socket_wrapper_t session);
ssize_t http_transaction_pull_request_body(http_transaction_t transaction, char *buffer, size_t max_length);
void http_transaction_no_payload_response(http_transaction_t transaction);
void http_transaction_payload_response(http_transaction_t transaction, const char *data, const size_t length);
void http_transaction_chunked_payload(http_transaction_t transaction, const char *data, const size_t length);
void http_transaction_end_chunked_payload(http_transaction_t transaction);
void http_transaction_start_sse(http_transaction_t transaction);
void http_transaction_publish_sse(http_transaction_t transaction, const char *event, char **data, size_t num_data);
void http_transaction_end_sse(http_transaction_t transaction);
void http_transaction_destroy(http_transaction_t transaction);

#ifdef __cplusplus
}
#endif

#endif