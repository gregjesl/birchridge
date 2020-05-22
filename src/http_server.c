#include "http_server.h"
#include <assert.h>

typedef struct http_context_struct
{
    http_server_t server;
    http_transaction_t transaction;
} *http_context_t;

void data_callback(socket_wrapper_t session)
{
    // Load the context
    http_context_t context = (http_context_t)session->context;

    // Check for a loaded transaction
    if(context->transaction == NULL) {
        
        // Header has not been parsed yet
        socket_wrapper_buffer(session);
        
        // Check for a full header
        const char *header_end = strstr(session->buffer->data, "\r\n\r\n");

        if(header_end != NULL) {

            // Full header found
            char *read_index = session->buffer->data;
            http_request_t header = http_parse_request(&read_index);
            if(header == NULL) {
                // Handle error
            }

            // Splice the buffer
            const size_t header_length = header_end - session->buffer->data + 4;
            socket_buffer_splice(session->buffer, header_length);

            // Create the transaction
            context->transaction = http_transaction_init(header, session);

            // Search for a callback
            const bool handled = http_handler_execute(context->server->handlers, context->transaction);
            if(!handled) {
                context->transaction->response->status_code = 404;
                context->transaction->response->content_length = 0;
                http_transaction_start_response(context->transaction);
            }

            // Verify the callback was successful
            if(!context->transaction->response_started) {
                if(context->transaction->response->status_code < 0 || context->transaction->response->content_length > 0) {
                    perror("Response not transmitted");
                    context->transaction->response->status_code = 500;
                    context->transaction->response->content_length = 0;
                    context->transaction->response->body_remaining = 0;
                }
                http_transaction_start_response(context->transaction);
                context->transaction->session->closure_requested = true;
            }

            if(context->transaction->request->body_remaining > 0) {
                perror("Request body not fully read");
                context->transaction->session->closure_requested = true;
            }

            // Check for keep-alive connection
            if(!context->transaction->request->keep_alive) {
                context->transaction->session->closure_requested = true;
            }

            http_transaction_destroy(context->transaction);
            context->transaction = NULL;
        }
    }
}

void closure_callback(socket_wrapper_t socket)
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
    socket_session_start(session, data_callback, closure_callback);
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