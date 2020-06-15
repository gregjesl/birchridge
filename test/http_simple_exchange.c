#include "http_server.h"
#include "macrothreading_condition.h"
#include "test.h"

const char *test_phrase = "GET / HTTP/1.1\r\n\r\n";
char *test_body = "Hello World!";
char *test_body_index = "\0";
const char *test_phrase_keep_alive = "GET / HTTP/1.1\r\nConnection: Keep-Alive\r\n\r\n";
const char *expected_response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nHello World!";

macrothread_condition_t callback_signal;
macrothread_condition_t closure_signal;

void server_callback(http_transaction_t transaction, void *context)
{
    transaction->response->status_code = 200;
    http_response_set_content_type(transaction->response, "text/plain");
    http_transaction_payload_response(transaction, test_body, strlen(test_body));
}

void client_data_callback(socket_wrapper_t session)
{
    if(socket_data_length(session->data) == strlen(expected_response)) {
        TEST_STRING_EQUAL(session->data->buffer, expected_response);
        socket_data_pop(session->data, strlen(expected_response));
        macrothread_condition_signal(callback_signal);
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
        socket_session_t client = socket_session_create(1024);
        TEST_NOT_NULL(client);

        client->data_callback = client_data_callback;
        client->hangup_callback = client_closure_callback;

        TEST_EQUAL(socket_session_connect(client, "127.0.0.1", 8081), SOCKET_OK);

        // Send the data
        for(size_t i = 0; i < strlen(test_phrase); i++) {
            TEST_EQUAL(socket_wrapper_write(client->socket, &test_phrase[i], 1), SOCKET_OK);
        }

        // Wait for the response
        macrothread_condition_wait(callback_signal);

        // Wait for the closure
        macrothread_condition_wait(closure_signal);
    }

    // Keep-alive
    socket_session_t client = socket_session_create(1024);
    TEST_NOT_NULL(client);
    client->data_callback = client_data_callback;
    TEST_EQUAL(socket_session_connect(client, "127.0.0.1", 8081), SOCKET_OK);
    for(size_t i = 0; i < 3; i++) {

        // Send the data
        for(size_t i = 0; i < strlen(test_phrase_keep_alive); i++) {
            TEST_EQUAL(socket_wrapper_write(client->socket, &test_phrase_keep_alive[i], 1), SOCKET_OK);
        }

        // Wait for the response
        macrothread_condition_wait(callback_signal);
    }

    socket_session_disconnect(client);

    http_server_stop(server);
    http_server_destroy(server);
}