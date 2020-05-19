#include "http_transaction.h"
#include <assert.h>

const char *response_code_ok = "OK\n";
const char *response_code_created = "Created";
const char *response_code_accepted = "Accepted";
const char *response_code_no_content = "No Content";
const char *response_code_bad_request = "Bad Request";
const char *response_code_unauthorized = "Unauthorized";
const char *response_code_forbidden = "Forbidden";
const char *response_code_not_found = "Not Found";

http_transaction_t http_transaction_init(http_request_t request, socket_session_t session)
{
    http_transaction_t result = (http_transaction_t)malloc(sizeof(struct http_transaction_struct));
    result->request = request;
    result->response = http_response_init(request->major_version, request->minor_version);
    result->session = session;
    result->response_started = false;
    return result;
}

void http_transaction_buffer_request_body(http_transaction_t transaction)
{
    char buffer[1024];
    ssize_t bytes_read = 0;
    while(transaction->session->socket->buffer->length < transaction->request->content_length) {
        bytes_read = http_transaction_read_request_body(transaction, buffer, 1024);
        if(bytes_read < 0) return;
        socket_buffer_write(transaction->session->socket->buffer, buffer, bytes_read);
    }
}

ssize_t http_transaction_read_request_body(http_transaction_t transaction, char *data, const size_t max_length)
{
    const size_t bytes_to_read = max_length > transaction->request->body_remaining ? transaction->request->body_remaining : max_length;
    ssize_t bytes_read = socket_wrapper_read(transaction->session->socket, data, bytes_to_read);
    if(bytes_read < 0) return bytes_read;
    assert(transaction->request->body_remaining >= bytes_read);
    transaction->request->body_remaining -= bytes_read;
    return (size_t)bytes_read;
}

void http_transaction_start_response(http_transaction_t transaction)
{
    // Write the beginning
    const char *beginning = "HTTP/";
    socket_session_write(transaction->session, beginning, strlen(beginning));

    {
        char version[5];
        sprintf(version, "%i.%i ", transaction->response->major_version, transaction->response->minor_version);
        socket_session_write(transaction->session, version, strlen(version));
    }

    {
        char status_code[5];
        sprintf(status_code, "%i ", transaction->response->status_code);
        socket_session_write(transaction->session, status_code, strlen(status_code));
    }
    switch (transaction->response->status_code)
    {
    case 200:
        socket_session_write(transaction->session, response_code_ok, strlen(response_code_ok));
        break;
    case 201:
        socket_session_write(transaction->session, response_code_created, strlen(response_code_created));
        break;
    case 202:
        socket_session_write(transaction->session, response_code_accepted, strlen(response_code_accepted));
        break;
    case 204:
        socket_session_write(transaction->session, response_code_no_content, strlen(response_code_no_content));
        break;
    default:
        // TODO
        break;
    }
    socket_session_write(transaction->session, "\n", 1);

    // Write the headers
    {
        http_header_t current = transaction->response->headers;
        http_header_t next = NULL;
        while(current != NULL) {
            socket_session_write(transaction->session, current->key, strlen(current->key));
            socket_session_write(transaction->session, ": ", 2);
            socket_session_write(transaction->session, current->value, strlen(current->value));
            socket_session_write(transaction->session, "\n", 1);
            next = current->next;
            current = next;
        }
    }

    // Write the empty line
    socket_session_write(transaction->session, "\n", 1);

    // Record the transmission
    transaction->response_started = true;
}

void http_transaction_send_response_body(http_transaction_t transaction, const char *data, const size_t length)
{
    assert(transaction->response_started);
    assert(transaction->response->body_remaining <= length);
    socket_session_write(transaction->session, data, length);
    transaction->response->body_remaining -= length;
}

void http_transaction_destroy(http_transaction_t transaction)
{
    http_response_destroy(transaction->response);
    free(transaction);
}