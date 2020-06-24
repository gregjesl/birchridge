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
const char *response_code_method_not_allowed = "Method Not Allowed";
const char *response_code_conflict = "Conflict";
const char *response_code_header_too_large = "Request Header Fields Too Large";
const char *response_code_internal_error = "Internal Server Error";

const char *content_length_key = "Content-Length";

const char *newline = "\r\n";

http_transaction_t http_transaction_init(http_request_t request, socket_wrapper_t session)
{
    http_transaction_t result = (http_transaction_t)malloc(sizeof(struct http_transaction_struct));
    result->request = request;
    result->response = http_response_init(request->major_version, request->minor_version);
    result->session = session;
    result->head_sent = false;
    result->response_complete = false;
    return result;
}

ssize_t http_transaction_pull_request_body(http_transaction_t transaction, char *buffer, size_t max_length)
{
    char *index = buffer;
    if(max_length == 0) return 0;
    if(transaction->request->body_remaining == 0) return 0;

    if(transaction->session->data->buffer > 0) {
        size_t bytes_to_copy = (transaction->session->data->buffer_length < max_length) ? transaction->session->data->buffer_length : max_length;
        if(bytes_to_copy > transaction->request->body_remaining) {
            bytes_to_copy = transaction->request->body_remaining;
        }
        if(buffer != NULL)
            memcpy(buffer, transaction->session->data->buffer, bytes_to_copy);
        socket_data_pop(transaction->session->data, bytes_to_copy);
        transaction->request->body_remaining -= bytes_to_copy;
        return bytes_to_copy;
    }

    const size_t bytes_to_read = transaction->request->body_remaining < max_length ? transaction->request->body_remaining : max_length;
    const ssize_t bytes_read = socket_wrapper_read(transaction->session, buffer, bytes_to_read, bytes_to_read);
    if(bytes_read > 0) {
        assert(transaction->request->body_remaining >= bytes_read);
        transaction->request->body_remaining -= bytes_read;
    }
    return bytes_read;
}

void __start_response(http_transaction_t transaction)
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
        if(transaction->response->status_text == NULL) {
            switch (transaction->response->status_code)
            {
            case 200:
                transaction->response->status_text = response_code_ok;
                break;
            case 201:
                transaction->response->status_text = response_code_created;
                break;
            case 202:
                transaction->response->status_text = response_code_accepted;
                break;
            case 204:
                transaction->response->status_text = response_code_no_content;
                break;
            case 400: 
                transaction->response->status_text = response_code_bad_request;
                break;
            case 401:
                transaction->response->status_text = response_code_unauthorized;
                break;
            case 403:
                transaction->response->status_text = response_code_forbidden;
                break;
            case 404:
                transaction->response->status_text = response_code_not_found;
                break;
            case 405:
                transaction->response->status_text = response_code_method_not_allowed;
                break;
            case 409:
                transaction->response->status_text = response_code_conflict;
                break;
            case 500:
            default:
                transaction->response->status_code = 500;
                transaction->response->status_text = response_code_internal_error;
                break;
            }
        }
        char status_code[5];
        sprintf(status_code, "%i ", transaction->response->status_code);
        socket_wrapper_write(transaction->session, status_code, strlen(status_code));
        socket_wrapper_write(transaction->session, transaction->response->status_text, strlen(transaction->response->status_text));
        socket_wrapper_write(transaction->session, newline, 2);
    }

    // Write the headers
    {
        key_value_pair_t current = *transaction->response->headers;
        key_value_pair_t next = NULL;
        while(current != NULL) {
            socket_wrapper_write(transaction->session, current->key, strlen(current->key));
            socket_wrapper_write(transaction->session, ": ", 2);
            socket_wrapper_write(transaction->session, current->value, strlen(current->value));
            socket_wrapper_write(transaction->session, newline, 2);
            next = current->next;
            current = next;
        }
    }

    // Write the empty line
    socket_wrapper_write(transaction->session, newline, 2);

    // Mark that the head has been sent
    transaction->head_sent = true;
}

void http_transaction_no_payload_response(http_transaction_t transaction)
{
    __start_response(transaction);
    assert(transaction->head_sent);
    transaction->response_complete = true;
}

void __set_body_length(http_response_t response, size_t length)
{
    assert(response->headers != NULL);
    key_value_pair_t current = *response->headers;
    key_value_pair_t next = NULL;
    if(current == NULL) {
        current = key_value_pair_init();
        key_value_linked_list_append(response->headers, current);
        key_value_pair_set_key(current, content_length_key);
    } else {
        while(strcmp(current->key, content_length_key) != 0) {
            next = current->next;
            current = next;
            if(current == NULL) {
                current = key_value_pair_init();
                key_value_linked_list_append(response->headers, current);
                key_value_pair_set_key(current, content_length_key);
                break;
            }
        }
    }

    // Determine the number of digits
    size_t digits = 1;
    size_t level = length / 10;
    while (level > 0) {
        digits++;
        level /= 10;
    }
    char *value = (char*)malloc((digits + 1) * sizeof(char));
    sprintf(value, "%lu", length);
    key_value_pair_set_value(current, value);
    free(value);
}

void http_transaction_payload_response(http_transaction_t transaction, const char *data, const size_t length)
{
    if(length == 0) {
        http_transaction_no_payload_response(transaction);
        return;
    }

    if(data == NULL) {
        transaction->response->status_code = 500;
        http_transaction_no_payload_response(transaction);
        return;
    }

    __set_body_length(transaction->response, length);
    __start_response(transaction);
    assert(transaction->head_sent);
    socket_wrapper_write(transaction->session, data, length);
    transaction->response_complete = true;
}

void __build_hex(size_t input, char *output)
{
    long quotient, remainder;
    int i, j = 0;
 
    quotient = input;
 
    while (quotient != 0)
    {
        remainder = quotient % 16;
        if (remainder < 10)
            output[j++] = 48 + remainder;
        else
            output[j++] = 55 + remainder;
        quotient = quotient / 16;
    }

    // Flip the string
    char *temp = (char*)malloc((j+2) * sizeof(char));
    for (i = j; i >= 0; i--)
        temp[j - i] = output[i];
    temp[j+1] = '\0';
    strcpy(output, temp);
    free(temp);
}

void http_transaction_chunked_payload(http_transaction_t transaction, const char *data, const size_t length)
{
    char hex[20];
    if(length == 0) {
        if(transaction->head_sent) {
            http_transaction_end_chunked_payload(transaction);
        } else {
            http_transaction_no_payload_response(transaction);
        }
        return;
    }

    if(data == NULL) {
        if(transaction->head_sent) {
            socket_wrapper_shutdown(transaction->session);
        } else {
            transaction->response->status_code = 500;
            http_transaction_no_payload_response(transaction);
        }
    }

    if(!transaction->head_sent) {
        key_value_pair_t current = key_value_pair_init();
        key_value_pair_set_key(current, "Transfer-Encoding");
        key_value_pair_set_value(current, "chunked");
        key_value_linked_list_append(transaction->response->headers, current);
        __start_response(transaction);
    }
    
    // Get the hex value
    __build_hex(length, hex);

    // Send the hex value
    socket_wrapper_write(transaction->session, hex, strlen(hex));
    socket_wrapper_write(transaction->session, newline, 2);
    socket_wrapper_write(transaction->session, data, length);
    socket_wrapper_write(transaction->session, newline, 2);
}

void http_transaction_end_chunked_payload(http_transaction_t transaction)
{
    if(!transaction->head_sent) {
        http_transaction_no_payload_response(transaction);
        return;
    }
    
    socket_wrapper_write(transaction->session, "0\r\n\r\n", 5);
    transaction->response_complete = true;
}

void http_transaction_start_sse(http_transaction_t transaction)
{
    transaction->response->status_code = 200;
    http_response_set_header(transaction->response, "Content-Type", "text/event-stream");
    __start_response(transaction);
    assert(transaction->head_sent);
}

void http_transaction_publish_sse(http_transaction_t transaction, const char *event, const char **data, size_t num_data)
{
    if(event == NULL && num_data == 0) return;

    if(event != NULL) {
        socket_wrapper_write(transaction->session, event, strlen(event));
        socket_wrapper_write(transaction->session, newline, 2);
    }

    for(size_t i = 0; i < num_data; i++) {
        socket_wrapper_write(transaction->session, data[i], strlen(data[i]));
        socket_wrapper_write(transaction->session, newline, 2);
    }

    socket_wrapper_write(transaction->session, newline, 2);
}

void http_transaction_end_sse(http_transaction_t transaction)
{
    transaction->response_complete = true;
}

void http_transaction_destroy(http_transaction_t transaction)
{
    http_response_destroy(transaction->response);
    free(transaction);
}