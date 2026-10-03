#include "buffer.h"
#include "greet.h"
#include "mctest.h"

static void test_greet(void)
{
    struct buffer b;
    buffer_init(&b);

    greet(buffer_writer(&b), "Chris");

    CHECK_STR(b.data, "Hello, Chris!");

    buffer_free(&b);
}

int main(void)
{
    RUN_TEST(test_greet);
    return test_summary();
}
