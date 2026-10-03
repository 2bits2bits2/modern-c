#include "generic_array.h"
#include "mctest.h"

DEFINE_ARRAY(int_array, int)
DEFINE_ARRAY(double_array, double)

static void test_int_array(void)
{
    struct int_array a;

    int_array_init(&a);
    for (int i = 0; i < 10; i++) {
        CHECK_TRUE(int_array_push(&a, i));
    }

    CHECK_INT(a.len, 10);
    for (int i = 0; i < 10; i++) {
        CHECK_INT(a.data[i], i);
    }

    int_array_free(&a);
}

static void test_double_array(void)
{
    struct double_array a;

    double_array_init(&a);
    CHECK_TRUE(double_array_push(&a, 1.5));
    CHECK_TRUE(double_array_push(&a, 2.5));

    CHECK_DOUBLE(a.data[0], 1.5, 1e-9);
    CHECK_DOUBLE(a.data[1], 2.5, 1e-9);

    double_array_free(&a);
}

int main(void)
{
    RUN_TEST(test_int_array);
    RUN_TEST(test_double_array);
    return test_summary();
}
