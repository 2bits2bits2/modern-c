#include "greeting.h"
#include "mctest.h"

static void test_greeting(void)
{
    char buf[64];

    greeting("Chris", buf, sizeof buf);
    CHECK_STR(buf, "Hello, Chris!");

    greeting("", buf, sizeof buf);
    CHECK_STR(buf, "Hello, !");
}

int main(void)
{
    RUN_TEST(test_greeting);
    return test_summary();
}
