#include "http_transaction.h"
#include <assert.h>

const char *response_code_ok = "OK";
const char *response_code_created = "Created";
const char *response_code_accepted = "Accepted";
const char *response_code_no_content = "No Content";
const char *response_code_bad_request = "Bad Request";
const char *response_code_unauthorized = "Unauthorized";
const char *response_code_forbidden = "Forbidden";
const char *response_code_not_found = "Not Found";
const char *response_code_header_too_large = "Request Header Fields Too Large";
const char *response_code_internal_error = "Internal Server Error";

http_transaction_t http_transaction_init(http_request_t request, socket_wrapper_t session)
{
    http_transaction_t result = (http_transaction_t)malloc(sizeof(struct http_transaction_struct));
    result->request = request;
    result->response = http_response_init(request->major_version, request->minor_version);
    result->session = session;
    result->response_started = false;
    return result;
}

void http_transaction_start_response(http_transaction_t transaction)
{
    // Write the beginning
    const char *beginning = "HTTP/";
    socket_wrapper_write(transaction->session, beginning, strlen(beginning));

    {
        char version[5];
        sprintf(version, "%i.%i ", transaction->response->major_version, transaction->response->minor_version);
        socket_wrapper_write(transaction->session, version, strlen(version));
    }

    {
        const char *response_text = NULL;
        switch (transaction->response->status_code)
        {
        case 200:
            response_text = response_code_ok;
            break;
        case 201:
            response_text = response_code_created;
            break;
        case 202:
            response_text = response_code_accepted;
            break;
        case 204:
            response_text = response_code_no_content;
            break;
        case 400: 
            response_text = response_code_bad_request;
            break;
        case 401:
            response_text = response_code_unauthorized;
            break;
        case 403:
            response_text = response_code_forbidden;
            break;
        case 404:
            response_text = response_code_not_found;
            break;
        case 500:
        default:
            transaction->response->status_code = 500;
            response_text = response_code_internal_error;
            break;
        }
        char status_code[5];
        sprintf(status_code, "%i ", transaction->response->status_code);
        socket_wrapper_write(transaction->session, status_code, strlen(status_code));
        socket_wrapper_write(transaction->session, response_text, strlen(response_text));
        socket_wrapper_write(transaction->session, "\r\n", 2);
    }

    // Write the headers
    {
        key_value_pair_t current = *transaction->response->headers;
        key_value_pair_t next = NULL;
        while(current != NULL) {
            socket_wrapper_write(transaction->session, current->key, strlen(current->key));
            socket_wrapper_write(transaction->session, ": ", 2);
            socket_wrapper_write(transaction->session, current->value, strlen(current->value));
            socket_wrapper_write(transaction->session, "\r\n", 2);
            next = current->next;
            current = next;
        }
    }

    // Write the empty line
    socket_wrapper_write(transaction->session, "\r\n", 2);

    // Record the transmission
    transaction->response_started = true;
}

void http_transaction_send_response_body(http_transaction_t transaction, const char *data, const size_t length)
{
    assert(transaction->response_started);
    assert(transaction->response->body_remaining >= length);
    socket_wrapper_write(transaction->session, data, length);
    transaction->response->body_remaining -= length;
}

void http_transaction_destroy(http_transaction_t transaction)
{
    http_response_destroy(transaction->response);
    free(transaction);
}