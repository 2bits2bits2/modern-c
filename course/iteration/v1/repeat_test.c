#include "repeat.h"
#include "mctest.h"

static void test_repeat(void)
{
    char buf[32];

    repeat("a", 3, buf, sizeof buf);

    CHECK_STR(buf, "aaa");
}

static void test_repeat_zero_times(void)
{
    char buf[32];

    repeat("a", 0, buf, sizeof buf);

    CHECK_STR(buf, "");
}

static void test_repeat_a_word(void)
{
    char buf[32];

    repeat("na", 4, buf, sizeof buf);

    CHECK_STR(buf, "nananana");
}

static void test_repeat_truncates_to_fit(void)
{
    char buf[4];

    repeat("ab", 5, buf, sizeof buf);

    CHECK_STR(buf, "aba");
}

int main(void)
{
    RUN_TEST(test_repeat);
    RUN_TEST(test_repeat_zero_times);
    RUN_TEST(test_repeat_a_word);
    RUN_TEST(test_repeat_truncates_to_fit);
    return test_summary();
}
