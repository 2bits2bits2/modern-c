#include "sum.h"
#include "mctest.h"

static void test_sum_of_numbers(void)
{
    int numbers[] = {1, 2, 3, 4, 5};

    CHECK_INT(sum(numbers, 5), 15);
}

static void test_sum_of_empty_array(void)
{
    CHECK_INT(sum(NULL, 0), 0);
}

static void test_sum_of_one(void)
{
    int numbers[] = {42};

    CHECK_INT(sum(numbers, 1), 42);
}

int main(void)
{
    RUN_TEST(test_sum_of_numbers);
    RUN_TEST(test_sum_of_empty_array);
    RUN_TEST(test_sum_of_one);
    return test_summary();
}
