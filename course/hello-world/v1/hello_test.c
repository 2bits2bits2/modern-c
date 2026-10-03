#include "hello.h"
#include "mctest.h"

static void test_hello(void)
{
    CHECK_STR(hello(), "Hello, world");
}

int main(void)
{
    RUN_TEST(test_hello);
    return test_summary();
}
