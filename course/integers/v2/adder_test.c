#include <limits.h>

#include "adder.h"
#include "mctest.h"

static void test_add(void)
{
    CHECK_INT(add(2, 2), 4);
}

static void test_add_checked_within_range(void)
{
    int got = 0;

    CHECK_TRUE(add_checked(2, 2, &got));
    CHECK_INT(got, 4);
}

static void test_add_checked_detects_overflow(void)
{
    int got = 0;

    CHECK_TRUE(!add_checked(INT_MAX, 1, &got));
}

static void test_add_checked_detects_underflow(void)
{
    int got = 0;

    CHECK_TRUE(!add_checked(INT_MIN, -1, &got));
}

int main(void)
{
    RUN_TEST(test_add);
    RUN_TEST(test_add_checked_within_range);
    RUN_TEST(test_add_checked_detects_overflow);
    RUN_TEST(test_add_checked_detects_underflow);
    return test_summary();
}
