#include "http_server.h"
#include "socket_data.h"
#include <assert.h>

typedef struct http_context_struct
{
    http_server_t server;
    http_transaction_t transaction;
} *http_context_t;

void __transmit_code(http_transaction_t transaction, int code)
{
    transaction->response->status_code = code;
    transaction->response->content_length = 0;
    transaction->response->body_remaining = 0;
    http_transaction_start_response(transaction);
    socket_wrapper_shutdown(transaction->session);
    http_transaction_destroy(transaction);
}

void __internal_server_error(http_transaction_t transaction)
{
    __transmit_code(transaction, 500);
}

void data_callback(socket_wrapper_t session)
{

    // Load the context
    http_context_t context = (http_context_t)session->context;

    start:

    // Check for a loaded transaction
    if(context->transaction == NULL) {
        
        // Check for a full header
        const char *header_end = strstr(session->data->buffer, "\r\n\r\n");
        if(header_end == NULL) {

            // Check for a too-large header
            if(session->data->buffer_length == socket_data_length(session->data)) {
                http_request_t req = http_request_init();
                req->major_version = 1;
                req->minor_version = 1;
                context->transaction = http_transaction_init(req, session);
                __transmit_code(context->transaction, 431);
                context->transaction = NULL;
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
            context->transaction = http_transaction_init(req, session);
            __internal_server_error(context->transaction);
            context->transaction = NULL;
            return;
        }
        socket_data_pop(session->data, read_index - session->data->buffer);

        // Create the transaction
        context->transaction = http_transaction_init(header, session);

        // Search for a callback
        const bool handled = http_handler_execute(context->server->handlers, context->transaction);
        if(!handled) {
            __transmit_code(context->transaction, 404);
            context->transaction = NULL;
            return;
        }
    }

    // Process the request body
    if(context->transaction->request->content_length > 0) {

        const size_t buffer_bytes = session->data->buffer_length - socket_data_length(session->data);
        const size_t bytes_to_read = buffer_bytes < context->transaction->request->content_length ? buffer_bytes : context->transaction->request->content_length;
        context->transaction->request->body_remaining -= bytes_to_read;

        if(context->transaction->request->body_callback != NULL) {
            context->transaction->request->body_callback(
                session->data->buffer,
                bytes_to_read,
                context->transaction->request->context
            );
        }

        socket_data_pop(session->data, bytes_to_read);
    }

    // Check for a full body read
    if(context->transaction->request->body_remaining > 0) {
        return;
    }

    // Check for a valid response
    if(context->transaction->response->status_code < 0 || 
        (context->transaction->response->content_length > 0 && context->transaction->response->body_callback == NULL)
    ) {
        __internal_server_error(context->transaction);
        context->transaction = NULL;
        return;
    }

    // Send the response header
    http_transaction_start_response(context->transaction);

    // Read the body
    char *shovel = (char*)malloc(session->data->buffer_length * sizeof(char));
    do
    {
        const size_t bytes_to_read = context->transaction->response->body_remaining > session->data->buffer_length ? session->data->buffer_length : context->transaction->response->body_remaining;
        const size_t bytes_read = context->transaction->response->body_callback(shovel, bytes_to_read, context->transaction->response->context);
        if(bytes_read > bytes_to_read) {
            // TODO: Error
        }
        http_transaction_send_response_body(context->transaction, shovel, bytes_read);
    } while (context->transaction->response->body_remaining > 0);
    free(shovel);

    // Check for keep-alive connection
    if(!context->transaction->request->keep_alive) {
        socket_wrapper_shutdown(context->transaction->session);
    }

    http_transaction_destroy(context->transaction);
    context->transaction = NULL;

    // Check for more data to process
    if(socket_data_length(session->data) > 0) {
        goto start;
    }
}

void finalize_callback(socket_wrapper_t socket)
{
    http_context_t context = (http_context_t)socket->context;
    if(context->transaction != NULL) {
        http_transaction_destroy(context->transaction);
    }
    free(context);
}

void connection_callback(socket_session_t session, void* context)
{
    http_context_t wrapper = (http_context_t)malloc(sizeof(struct http_context_struct));
    wrapper->server = (http_server_t)context;
    wrapper->transaction = NULL;
    session->socket->context = wrapper;
    session->data_callback = data_callback;
    session->finalize_callback = finalize_callback;
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