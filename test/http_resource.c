#include "http_resource.h"
#include "test.h"

int main(void)
{
    const char *simple = "/";
    http_resource_t result = http_resource_parse(simple);
    TEST_NOT_NULL(result);
    TEST_STRING_EQUAL(result->path, simple);
    TEST_EQUAL(key_value_linked_list_count(result->query_parameters), 0);
    http_resource_destroy(result);

    const char *one_param = "/test.html?key=value";
    result = http_resource_parse(one_param);
    TEST_STRING_EQUAL(result->path, "/test.html");
    TEST_EQUAL(key_value_linked_list_count(result->query_parameters), 1);
    TEST_STRING_EQUAL(result->query_parameters[0]->key, "key");
    TEST_STRING_EQUAL(key_value_find(result->query_parameters, "key")->value, "value");
    http_resource_destroy(result);

    const char *two_param = "/test2.html?birch=ridge&hello=world";
    result = http_resource_parse(two_param);
    TEST_STRING_EQUAL(result->path, "/test2.html");
    TEST_EQUAL(key_value_linked_list_count(result->query_parameters), 2);
    TEST_STRING_EQUAL(result->query_parameters[0]->key, "birch");
    TEST_STRING_EQUAL(key_value_find(result->query_parameters, "birch")->value, "ridge");
    TEST_STRING_EQUAL(result->query_parameters[0]->next->key, "hello");
    TEST_STRING_EQUAL(key_value_find(result->query_parameters, "hello")->value, "world");
    http_resource_destroy(result);
}