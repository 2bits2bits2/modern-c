#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include "writer.h"

/*
 * A sleeper is anything that can wait. In production it wraps sleep(3); in a
 * test it is a spy that records what it was asked to do.
 */
struct sleeper {
    void (*sleep)(void *ctx, int seconds);
    void *ctx;
};

void countdown(struct writer out, struct sleeper s);

#endif /* COUNTDOWN_H */
