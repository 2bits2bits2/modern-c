#include "blind.h"

int blind_level(const struct clock *clock, long start_ms, int level_minutes)
{
    long elapsed = clock_now_ms(clock) - start_ms;
    long level_ms = (long)level_minutes * 60L * 1000L;

    if (elapsed < 0) {
        return 0;
    }

    return (int)(elapsed / level_ms);
}
