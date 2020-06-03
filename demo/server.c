#include "http_server.h"
#include "stdio.h"

const char *body = "Hello World!";

size_t body_callback(char *buffer, size_t length, void *context)
{
    memcpy(buffer, body, strlen(body));
    return strlen(body);
}

void callback(http_transaction_t transaction, void *context)
{
    transaction->response->status_code = 200;
    http_response_set_body_length(transaction->response, strlen(body));
    // transaction->response->body_callback = body_callback;
}

int main(int argc, char *argv[])
{
    if(argc != 2) {
        fprintf(stderr, "Expecting port as arguement");
        exit(1);
    }

    int port = 0;
    sscanf("%i", argv[1], port);
    if(port == 0) {
        port = 8081;
    }

    http_server_t server = http_server_init();
    http_server_add_handler(server, "/", callback, NULL);
    http_server_start(server, port, 5);
    printf("Press any key to stop...");
    getchar();
    http_server_stop(server);
    http_server_destroy(server);
}