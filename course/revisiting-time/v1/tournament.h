#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include "clock.h"
#include "scheduler.h"

struct tournament {
    struct fake_clock fake;
    struct clock clock;
    struct scheduler scheduler;
    int alerts;
};

/* Schedule one alert at the end of each level. */
void tournament_start(struct tournament *tournament, int levels,
                      int level_minutes);

/* Move virtual time forward and fire anything that is due. */
void tournament_advance(struct tournament *tournament, long ms);

int tournament_alerts(const struct tournament *tournament);

#endif /* TOURNAMENT_H */
