#include "clock.h"
#include "mctest.h"
#include "scheduler.h"

struct counter {
    int fired;
};

static void on_fire(void *ctx)
{
    struct counter *counter = ctx;

    counter->fired++;
}

static void test_callbacks_fire_in_time_order(void)
{
    struct fake_clock fake = {.now_ms = 0};
    struct clock clock = fake_clock_as_clock(&fake);
    struct scheduler scheduler;
    struct counter counter = {0};

    scheduler_init(&scheduler, &clock, 0);
    CHECK_TRUE(scheduler_at(&scheduler, 10, on_fire, &counter));
    CHECK_TRUE(scheduler_at(&scheduler, 20, on_fire, &counter));

    CHECK_INT(scheduler_run(&scheduler), 0);
    CHECK_INT(counter.fired, 0);

    fake_clock_advance(&fake, 10);
    CHECK_INT(scheduler_run(&scheduler), 1);
    CHECK_INT(counter.fired, 1);

    /* Running again before anything else is due fires nothing. */
    CHECK_INT(scheduler_run(&scheduler), 0);

    fake_clock_advance(&fake, 10);
    CHECK_INT(scheduler_run(&scheduler), 1);
    CHECK_INT(counter.fired, 2);

    /* Each callback fires exactly once. */
    fake_clock_advance(&fake, 1000);
    CHECK_INT(scheduler_run(&scheduler), 0);
    CHECK_INT(counter.fired, 2);
}

int main(void)
{
    RUN_TEST(test_callbacks_fire_in_time_order);
    return test_summary();
}
