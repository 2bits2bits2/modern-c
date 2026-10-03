#include "mctest.h"
#include "roman.h"

static void test_known_numerals(void)
{
    char buf[32];

    to_roman(1, buf, sizeof buf);
    CHECK_STR(buf, "I");
    to_roman(4, buf, sizeof buf);
    CHECK_STR(buf, "IV");
    to_roman(9, buf, sizeof buf);
    CHECK_STR(buf, "IX");
    to_roman(1984, buf, sizeof buf);
    CHECK_STR(buf, "MCMLXXXIV");
    to_roman(3999, buf, sizeof buf);
    CHECK_STR(buf, "MMMCMXCIX");
}

static void test_out_of_range(void)
{
    char buf[32];

    CHECK_INT(to_roman(0, buf, sizeof buf), -1);
    CHECK_INT(to_roman(4000, buf, sizeof buf), -1);
}

static void test_from_roman(void)
{
    CHECK_INT(from_roman("I"), 1);
    CHECK_INT(from_roman("MCMLXXXIV"), 1984);
    CHECK_INT(from_roman("nonsense"), -1);
}

/*
 * A property: for every number in range, converting to Roman and back gives
 * the original. That is one test standing in for thousands.
 */
static void test_round_trip_property(void)
{
    char buf[32];

    for (int n = 1; n <= 3999; n++) {
        CHECK_INT(to_roman(n, buf, sizeof buf) > 0, 1);
        CHECK_INT(from_roman(buf), n);
    }
}

int main(void)
{
    RUN_TEST(test_known_numerals);
    RUN_TEST(test_out_of_range);
    RUN_TEST(test_from_roman);
    RUN_TEST(test_round_trip_property);
    return test_summary();
}
