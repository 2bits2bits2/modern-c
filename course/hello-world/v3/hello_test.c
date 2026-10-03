#include "hello.h"
#include "mctest.h"

static void test_hello_to_a_person(void)
{
    char buf[64];

    hello("Chris", buf, sizeof buf);

    CHECK_STR(buf, "Hello, Chris");
}

static void test_empty_name_defaults_to_world(void)
{
    char buf[64];

    hello("", buf, sizeof buf);

    CHECK_STR(buf, "Hello, World");
}

int main(void)
{
    RUN_TEST(test_hello_to_a_person);
    RUN_TEST(test_empty_name_defaults_to_world);
    return test_summary();
}
