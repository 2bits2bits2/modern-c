#ifndef PLAYER_STORE_H
#define PLAYER_STORE_H

#include <stddef.h>

#define PLAYER_NAME_MAX 64

struct player {
    char name[PLAYER_NAME_MAX];
    int wins;
};

struct player_store {
    struct player *players;
    size_t len;
    size_t cap;
};

void player_store_init(struct player_store *store);
void player_store_free(struct player_store *store);

/* Record a win for name, creating the player if necessary. */
void player_store_record_win(struct player_store *store, const char *name);

/* Look up a player's wins. Returns true and writes *out if found. */
bool player_store_get_wins(const struct player_store *store, const char *name,
                           int *out);

/* Sort players by wins, highest first. */
void player_store_sort_by_wins(struct player_store *store);

#endif /* PLAYER_STORE_H */
