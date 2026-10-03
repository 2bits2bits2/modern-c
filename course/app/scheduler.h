#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdbool.h>
#include <stddef.h>

#include "clock.h"

#define SCHEDULER_MAX 64

struct scheduled {
    long due_ms;
    void (*fn)(void *ctx);
    void *ctx;
    bool fired;
};

/*
 * A scheduler fires callbacks once their due time has passed, according to an
 * injected clock. It never sleeps: something else calls scheduler_run when it
 * wants due work to happen. In production that is the server's event loop; in
 * tests it is after advancing a fake clock.
 */
struct scheduler {
    const struct clock *clock;
    long start_ms;
    struct scheduled items[SCHEDULER_MAX];
    size_t len;
};

void scheduler_init(struct scheduler *scheduler, const struct clock *clock,
                    long start_ms);

/* Schedule fn to fire offset_ms after the start time. */
bool scheduler_at(struct scheduler *scheduler, long offset_ms,
                  void (*fn)(void *ctx), void *ctx);

/* Fire every callback whose due time has arrived. Returns how many fired. */
size_t scheduler_run(struct scheduler *scheduler);

#endif /* SCHEDULER_H */
