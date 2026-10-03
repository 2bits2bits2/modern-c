#include "describe.h"
#include "mctest.h"

static void test_describe(void)
{
    int i = 42;
    long l = 42;
    double d = 42;
    const char *s = "hello";

    CHECK_STR(describe(i), "int");
    CHECK_STR(describe(l), "long");
    CHECK_STR(describe(d), "double");
    CHECK_STR(describe(s), "string");
    CHECK_STR(describe(1.5f), "float");
}

int main(void)
{
    RUN_TEST(test_describe);
    return test_summary();
}
