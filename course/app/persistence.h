#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <stdbool.h>

#include "player_store.h"

/*
 * The on-disk format is JSON Lines: one flat JSON object per line, like
 *
 *   {"name":"Pepper","wins":10}
 *   {"name":"Floyd","wins":7}
 *
 * One object per line means we can reuse the flat parser for loading.
 */

/* Write every player to path, one JSON object per line. */
bool player_store_save(const struct player_store *store, const char *path);

/*
 * Load players from path into store (which should be empty). A missing file
 * is treated as an empty store and returns true. Returns false on a read
 * error.
 */
bool player_store_load(struct player_store *store, const char *path);

#endif /* PERSISTENCE_H */
