#include "http_server.h"
#include "test.h"

const char *test_phrase = "GET / HTTP/1.1\r\n\r\n";
const char *test_phrase_keep_alive = "GET / HTTP/1.1\r\nConnection: Keep-Alive\r\n\r\n";
const char *expected_response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello World!";

macrothread_condition_t callback_signal;
macrothread_condition_t closure_signal;

void server_callback(http_transaction_t transaction, void *context)
{
    const char *body = "Hello World!";
    transaction->response->status_code = 200;
    http_response_set_body_length(transaction->response, strlen(body));
    http_transaction_start_response(transaction);
    http_transaction_send_response_body(transaction,body, strlen(body));
}

void client_data_callback(socket_wrapper_t session)
{
    socket_wrapper_buffer(session);
    if(session->buffer->length == strlen(expected_response)) {
        TEST_STRING_EQUAL(session->buffer->data, expected_response);
        macrothread_condition_signal(callback_signal);
        socket_buffer_splice(session->buffer, session->buffer->length);
    }
}

void client_closure_callback(socket_wrapper_t session)
{
    macrothread_condition_signal(closure_signal);
}

int main(int argc, char *argv[])
{
    callback_signal = macrothread_condition_init();
    closure_signal = macrothread_condition_init();

    // Create the server
    http_server_t server = http_server_init();
    http_server_add_handler(server, "/", server_callback, NULL);
    http_server_start(server, 8081, 5);

    // Create the client
    for(size_t i = 0; i < 3; i++) {
        socket_session_t client = socket_session_connect("127.0.0.1", 8081);
        TEST_NOT_NULL(client);

        // Start monitoring
        socket_session_start(client, client_data_callback, client_closure_callback);

        // Send the data
        for(size_t i = 0; i < strlen(test_phrase); i++) {
            socket_wrapper_write(client->socket, &test_phrase[i], 1);
        }

        // Wait for the response
        macrothread_condition_wait(callback_signal);

        // Wait for the closure
        macrothread_condition_wait(closure_signal);
    }

    // Keep-alive
    socket_session_t client = socket_session_connect("127.0.0.1", 8081);
    TEST_NOT_NULL(client);
    socket_session_start(client, client_data_callback, NULL);
    for(size_t i = 0; i < 3; i++) {

        // Send the data
        for(size_t i = 0; i < strlen(test_phrase_keep_alive); i++) {
            socket_wrapper_write(client->socket, &test_phrase_keep_alive[i], 1);
        }

        // Wait for the response
        macrothread_condition_wait(callback_signal);
    }

    socket_session_stop(client);
    socket_session_close(client);

    http_server_stop(server);
    http_server_destroy(server);
}