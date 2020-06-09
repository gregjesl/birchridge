#include "http_server.h"
#include "socket_data.h"
#include <assert.h>

void __transmit_code(http_transaction_t transaction, int code)
{
    transaction->response->status_code = code;
    http_transaction_no_payload_response(transaction);
    socket_wrapper_shutdown(transaction->session);
    http_transaction_destroy(transaction);
}

void __internal_server_error(http_transaction_t transaction)
{
    __transmit_code(transaction, 500);
}

void data_callback(socket_wrapper_t session)
{
    // Check for a full header
    const char *header_end = strstr(session->data->buffer, "\r\n\r\n");
    if(header_end == NULL) {

        // Check for a too-large header
        if(session->data->buffer_length == socket_data_length(session->data)) {
            http_request_t req = http_request_init();
            req->major_version = 1;
            req->minor_version = 1;
            http_transaction_t transaction = http_transaction_init(req, session);
            __transmit_code(transaction, 431);
            return;
        } else {
            return;
        }
    }

    // Full header found
    char *read_index = session->data->buffer;
    http_request_t header = http_parse_request(&read_index);
    if(header == NULL) {
        http_request_t req = http_request_init();
        req->major_version = 1;
        req->minor_version = 1;
        http_transaction_t transaction = http_transaction_init(req, session);
        __internal_server_error(transaction);
        return;
    }

    // Remove the header from the buffer
    socket_data_pop(session->data, read_index - session->data->buffer);

    // Create the transaction
    http_transaction_t transaction = http_transaction_init(header, session);

    // Search for a callback
    http_server_t server = (http_server_t)session->context;
    const bool handled = http_handler_execute(server->handlers, transaction);
    if(!handled) {
        __transmit_code(transaction, 404);
        return;
    }

    // Check for an unpushed response
    if(!transaction->head_sent) {
        http_transaction_no_payload_response(transaction);
    }

    // Check for an unread body
    while(header->body_remaining > 0) {
        if(http_transaction_pull_request_body(transaction, NULL, header->body_remaining) < 0) {
            transaction->request->keep_alive = false;
            break;
        }
    }
    
    // Check for keep-alive connection
    if(!transaction->request->keep_alive) {
        socket_wrapper_shutdown(session);
    }

    http_transaction_destroy(transaction);

    // Check for more data to process
    if(socket_data_length(session->data) > 0) {
        data_callback(session);
    }
}

void connection_callback(socket_session_t session, void* context)
{
    session->socket->context = context;
    session->data_callback = data_callback;
}

http_server_t http_server_init()
{
    http_server_t result = (http_server_t)malloc(sizeof(struct http_server_struct));
    result->handlers = NULL;
    result->listener = NULL;
    return result;
}

http_handler_t http_server_add_handler(http_server_t server, const char *path, http_transaction_callback callback, void *context)
{
    http_handler_t result = http_handler_init(path, callback, context);
    http_handler_add(&server->handlers, result);
    return result;
}

void http_server_remove_handler(http_server_t server, http_handler_t handler)
{
    http_handler_remove(&server->handlers, handler);
    http_handler_destroy(handler);
}

void http_server_start(http_server_t server, int port, int queue)
{
    assert(server->listener == NULL);
    server->listener = socket_listener_start(port, queue, connection_callback, (void*)server);
}

void http_server_stop(http_server_t server)
{
    socket_listener_stop(server->listener);
    server->listener = NULL;
}

void http_server_destroy(http_server_t server)
{
    if(server->listener != NULL) {
        http_server_stop(server);
    }

    http_handler_t current = server->handlers;
    http_handler_t next = NULL;
    while(current != NULL) {
        next = current->next;
        http_handler_destroy(current);
        current = next;
    }
}