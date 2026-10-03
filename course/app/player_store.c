#include "player_store.h"

#include <stdlib.h>
#include <string.h>

static void copy_name(char *dst, const char *src)
{
    size_t i = 0;

    for (; src[i] != '\0' && i + 1 < PLAYER_NAME_MAX; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

void player_store_init(struct player_store *store)
{
    store->players = NULL;
    store->len = 0;
    store->cap = 0;
}

void player_store_free(struct player_store *store)
{
    free(store->players);
    store->players = NULL;
    store->len = 0;
    store->cap = 0;
}

static struct player *find(struct player_store *store, const char *name)
{
    for (size_t i = 0; i < store->len; i++) {
        if (strcmp(store->players[i].name, name) == 0) {
            return &store->players[i];
        }
    }
    return NULL;
}

void player_store_record_win(struct player_store *store, const char *name)
{
    struct player *existing = find(store, name);

    if (existing != NULL) {
        existing->wins++;
        return;
    }

    if (store->len == store->cap) {
        size_t cap = store->cap == 0 ? 4 : store->cap * 2;
        struct player *players = realloc(store->players, cap * sizeof *players);

        if (players == NULL) {
            return;
        }
        store->players = players;
        store->cap = cap;
    }

    copy_name(store->players[store->len].name, name);
    store->players[store->len].wins = 1;
    store->len++;
}

bool player_store_get_wins(const struct player_store *store, const char *name,
                           int *out)
{
    for (size_t i = 0; i < store->len; i++) {
        if (strcmp(store->players[i].name, name) == 0) {
            if (out != NULL) {
                *out = store->players[i].wins;
            }
            return true;
        }
    }
    return false;
}

static int compare_by_wins_desc(const void *a, const void *b)
{
    const struct player *left = a;
    const struct player *right = b;

    return right->wins - left->wins;
}

void player_store_sort_by_wins(struct player_store *store)
{
    qsort(store->players, store->len, sizeof store->players[0],
          compare_by_wins_desc);
}
