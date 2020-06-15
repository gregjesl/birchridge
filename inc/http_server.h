#ifndef BIRCHRIDGE_HTTP_SERVER_H
#define BIRCHRIDGE_HTTP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "http_handler.h"
#include "socket_listener.h"

typedef struct http_server_struct
{
    http_handler_t handlers;
    socket_listener_t listener;
} *http_server_t;

http_server_t http_server_init();
http_handler_t http_server_add_handler(http_server_t server, const char *path, http_transaction_callback callback, void *context);
void http_server_remove_handler(http_server_t server, http_handler_t handler);
void http_server_start(http_server_t server, int port, int queue);
void http_server_stop(http_server_t server);
void http_server_destroy(http_server_t server);

#ifdef __cplusplus
}
#endif

#endif