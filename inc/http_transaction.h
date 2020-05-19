#ifndef BIRCHRIDGE_HTTP_TRANSACTION_H
#define BIRCHRIDGE_HTTP_TRANSACTION_H

#include "http_request.h"
#include "http_response_t.h"
#include "socket_session.h"

typedef struct http_transaction_struct
{
    http_request_t request;
    http_response_t response;
    socket_session_t session;
} *http_transaction_t;

#endif