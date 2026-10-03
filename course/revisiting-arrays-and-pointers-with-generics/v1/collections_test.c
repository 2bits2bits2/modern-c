#include "collections.h"
#include "mctest.h"

static struct int_array make_ints(const int *values, size_t n)
{
    struct int_array a;

    int_array_init(&a);
    for (size_t i = 0; i < n; i++) {
        int_array_push(&a, values[i]);
    }

    return a;
}

static void test_reduce(void)
{
    const int values[] = {1, 2, 3, 4};
    struct int_array a = make_ints(values, 4);

    CHECK_INT(reduce_sum(&a), 10);

    int_array_free(&a);
}

static void test_map(void)
{
    const int values[] = {1, 2, 3};
    struct int_array a = make_ints(values, 3);
    struct int_array doubled = map_double_all(&a);

    CHECK_INT(doubled.len, 3);
    CHECK_INT(doubled.data[0], 2);
    CHECK_INT(doubled.data[1], 4);
    CHECK_INT(doubled.data[2], 6);

    int_array_free(&doubled);
    int_array_free(&a);
}

static void test_filter(void)
{
    const int values[] = {1, 2, 3, 4, 5, 6};
    struct int_array a = make_ints(values, 6);
    struct int_array evens = filter_even(&a);

    CHECK_INT(evens.len, 3);
    CHECK_INT(evens.data[0], 2);
    CHECK_INT(evens.data[1], 4);
    CHECK_INT(evens.data[2], 6);

    int_array_free(&evens);
    int_array_free(&a);
}

int main(void)
{
    RUN_TEST(test_reduce);
    RUN_TEST(test_map);
    RUN_TEST(test_filter);
    return test_summary();
}
