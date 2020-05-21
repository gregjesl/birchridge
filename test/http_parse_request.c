#include "http_request.h"
#include "test.h"

int main(void)
{
    char *basic = "GET /index.html HTTP/1.1\nContent-Length: 123\n\n\0";
    http_request_t result = http_parse_request(&basic);
    TEST_NOT_NULL(result);
    TEST_EQUAL(result->method, HTTP_METHOD_GET);
    TEST_STRING_EQUAL(result->path, "/index.html");
    TEST_EQUAL(result->major_version, 1);
    TEST_EQUAL(result->minor_version, 1);
    TEST_NOT_NULL(result->headers);
    TEST_EQUAL(key_value_linked_list_count(result->headers), 1);
    TEST_STRING_EQUAL(result->headers[0]->key, "Content-Length\0");
    TEST_STRING_EQUAL(result->headers[0]->value, "123\0");
    TEST_EQUAL(result->content_length, 123);
    TEST_NULL(result->headers[0]->next);
    TEST_EQUAL(strlen(basic), 0);
    http_request_destroy(result);
}