#include "hello.h"
#include "mctest.h"

static void test_hello_to_a_person(void)
{
    char buf[64];

    hello("Chris", buf, sizeof buf);

    CHECK_STR(buf, "Hello, Chris");
}

int main(void)
{
    RUN_TEST(test_hello_to_a_person);
    return test_summary();
}
