#ifndef BLIND_H
#define BLIND_H

#include "clock.h"

/*
 * The current blind level for a game, based on how long ago it started and
 * how long each level lasts. Level 0 is the first level.
 */
int blind_level(const struct clock *clock, long start_ms, int level_minutes);

#endif /* BLIND_H */
