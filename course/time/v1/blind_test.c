#include "blind.h"
#include "clock.h"
#include "mctest.h"

static void test_blind_levels(void)
{
    struct fake_clock fake = {.now_ms = 10 * 1000};
    struct clock clock = fake_clock_as_clock(&fake);

    /* The game started at 10s; each level lasts one minute. */
    CHECK_INT(blind_level(&clock, 10 * 1000, 1), 0);

    fake_clock_advance(&fake, 30 * 1000); /* 40s in */
    CHECK_INT(blind_level(&clock, 10 * 1000, 1), 0);

    fake_clock_advance(&fake, 30 * 1000); /* 60s in: level 1 */
    CHECK_INT(blind_level(&clock, 10 * 1000, 1), 1);

    fake_clock_advance(&fake, 59 * 1000); /* 119s in: still level 1 */
    CHECK_INT(blind_level(&clock, 10 * 1000, 1), 1);

    fake_clock_advance(&fake, 1 * 1000); /* 120s in: level 2 */
    CHECK_INT(blind_level(&clock, 10 * 1000, 1), 2);
}

static void test_before_the_start_is_level_zero(void)
{
    struct fake_clock fake = {.now_ms = 0};
    struct clock clock = fake_clock_as_clock(&fake);

    CHECK_INT(blind_level(&clock, 5000, 1), 0);
}

int main(void)
{
    RUN_TEST(test_blind_levels);
    RUN_TEST(test_before_the_start_is_level_zero);
    return test_summary();
}
