#include "clock.h"

#include <time.h>

static long system_now_ms(void *ctx)
{
    struct timespec ts;

    (void)ctx;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long)ts.tv_sec * 1000L + (long)ts.tv_nsec / 1000000L;
}

long clock_now_ms(const struct clock *clock)
{
    return clock->now_ms(clock->ctx);
}

struct clock clock_system(void)
{
    return (struct clock){.now_ms = system_now_ms, .ctx = NULL};
}

long fake_clock_now(void *ctx)
{
    struct fake_clock *fake = ctx;

    return fake->now_ms;
}

struct clock fake_clock_as_clock(struct fake_clock *fake)
{
    return (struct clock){.now_ms = fake_clock_now, .ctx = fake};
}

void fake_clock_advance(struct fake_clock *fake, long ms)
{
    fake->now_ms += ms;
}
