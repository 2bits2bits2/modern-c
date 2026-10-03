#include "scheduler.h"

void scheduler_init(struct scheduler *scheduler, const struct clock *clock,
                    long start_ms)
{
    scheduler->clock = clock;
    scheduler->start_ms = start_ms;
    scheduler->len = 0;
}

bool scheduler_at(struct scheduler *scheduler, long offset_ms,
                  void (*fn)(void *ctx), void *ctx)
{
    if (scheduler->len >= SCHEDULER_MAX) {
        return false;
    }

    scheduler->items[scheduler->len] = (struct scheduled){
        .due_ms = scheduler->start_ms + offset_ms,
        .fn = fn,
        .ctx = ctx,
        .fired = false,
    };
    scheduler->len++;
    return true;
}

size_t scheduler_run(struct scheduler *scheduler)
{
    long now = clock_now_ms(scheduler->clock);
    size_t fired = 0;

    for (size_t i = 0; i < scheduler->len; i++) {
        struct scheduled *item = &scheduler->items[i];

        if (!item->fired && now >= item->due_ms) {
            item->fired = true;
            item->fn(item->ctx);
            fired++;
        }
    }

    return fired;
}
