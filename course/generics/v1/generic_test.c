#include "generic.h"
#include "mctest.h"

static void test_max_picks_the_type(void)
{
    CHECK_INT(max(3, 7), 7);
    CHECK_DOUBLE(max(2.5, 1.5), 2.5, 1e-9);
}

static void test_swap(void)
{
    int a = 1;
    int b = 2;

    swap(a, b);

    CHECK_INT(a, 2);
    CHECK_INT(b, 1);
}

int main(void)
{
    RUN_TEST(test_max_picks_the_type);
    RUN_TEST(test_swap);
    return test_summary();
}
