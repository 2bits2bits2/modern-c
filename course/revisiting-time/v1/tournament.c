#include "tournament.h"

static void on_alert(void *ctx)
{
    struct tournament *tournament = ctx;

    tournament->alerts++;
}

void tournament_start(struct tournament *tournament, int levels,
                      int level_minutes)
{
    tournament->fake.now_ms = 0;
    tournament->clock = fake_clock_as_clock(&tournament->fake);
    tournament->alerts = 0;

    scheduler_init(&tournament->scheduler, &tournament->clock, 0);
    for (int i = 1; i <= levels; i++) {
        scheduler_at(&tournament->scheduler,
                     (long)i * level_minutes * 60L * 1000L, on_alert,
                     tournament);
    }
}

void tournament_advance(struct tournament *tournament, long ms)
{
    fake_clock_advance(&tournament->fake, ms);
    scheduler_run(&tournament->scheduler);
}

int tournament_alerts(const struct tournament *tournament)
{
    return tournament->alerts;
}
