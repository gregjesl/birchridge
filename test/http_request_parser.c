#include "http_request_parser.h"
#include "test.h"

int main(void)
{
    const char *test_line = "one\ntwo\nthree\n\n\0";
    const char *index = test_line;
    http_request_parser parser = http_request_parser_init(5);
    size_t bytes_processed = 0;
    do
    {
        bytes_processed = http_request_parse(parser, index);
        index += bytes_processed;
    } while (bytes_processed > 0);
    
    TEST_STRING_EQUAL(parser->lines[0], "one\n");
    TEST_STRING_EQUAL(parser->lines[1], "two\n");
    TEST_STRING_EQUAL(parser->lines[2], "three\n");
    TEST_STRING_EQUAL(parser->lines[3], "\n");
    TEST_EQUAL(strlen(parser->lines[4]), 0);

    http_request_parser_destroy(parser);
}