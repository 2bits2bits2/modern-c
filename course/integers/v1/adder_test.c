#include "adder.h"
#include "mctest.h"

static void test_add(void)
{
    CHECK_INT(add(2, 2), 4);
}

static void test_add_with_zero(void)
{
    CHECK_INT(add(7, 0), 7);
}

static void test_add_negative(void)
{
    CHECK_INT(add(-3, 5), 2);
}

int main(void)
{
    RUN_TEST(test_add);
    RUN_TEST(test_add_with_zero);
    RUN_TEST(test_add_negative);
    return test_summary();
}
