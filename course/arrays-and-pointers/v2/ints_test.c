#include "ints.h"
#include "mctest.h"

static void test_append_single(void)
{
    struct ints xs = ints_new();

    CHECK_TRUE(ints_append(&xs, 7));
    CHECK_INT(xs.len, 1);
    CHECK_INT(xs.data[0], 7);

    ints_free(&xs);
}

static void test_append_many_grows(void)
{
    struct ints xs = ints_new();

    for (int i = 0; i < 100; i++) {
        CHECK_TRUE(ints_append(&xs, i));
    }

    CHECK_INT(xs.len, 100);
    for (int i = 0; i < 100; i++) {
        CHECK_INT(xs.data[i], i);
    }

    ints_free(&xs);
}

int main(void)
{
    RUN_TEST(test_append_single);
    RUN_TEST(test_append_many_grows);
    return test_summary();
}
