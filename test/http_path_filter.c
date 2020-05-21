#include "http_resource.h"
#include "test.h"

int main(void)
{
    TEST_TRUE(http_path_filter("/", "/"));
    TEST_TRUE(http_path_filter("/test", "/test"));
    TEST_TRUE(http_path_filter("/test/path", "/test/path"));
    TEST_TRUE(http_path_filter("/test/path", "/*/path"));
    TEST_TRUE(http_path_filter("/test/path", "/test/*"));
    TEST_TRUE(http_path_filter("/test/path", "/*/*"));
    TEST_TRUE(http_path_filter("/test/path", "/test/#"));
    TEST_TRUE(http_path_filter("/test/path", "/*/#"));
    TEST_TRUE(http_path_filter("/test/path", "/#"));

    TEST_FALSE(http_path_filter("/test/path", "/"));
    TEST_FALSE(http_path_filter("/test/path", "/different/path"));
    TEST_FALSE(http_path_filter("/test/path", "/path/test"));
    TEST_FALSE(http_path_filter("/test/path", "/*/test"));
    TEST_FALSE(http_path_filter("/test/path", "/*/*/#"));
}