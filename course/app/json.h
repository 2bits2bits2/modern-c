#ifndef JSON_H
#define JSON_H

#include <stdbool.h>
#include <stddef.h>

#include "player_store.h"

/* Encode every player as a JSON array into buf (NUL-terminated). */
void json_encode_league(const struct player_store *store, char *buf,
                        size_t cap);

/* Encode a single player as a flat JSON object. */
void json_encode_player(const struct player *player, char *buf, size_t cap);

/*
 * Minimal parsing for flat JSON objects. Neither function is a general JSON
 * parser; they are enough for the request bodies a small service receives.
 */
bool json_get_string(const char *json, const char *key, char *out, size_t cap);
bool json_get_int(const char *json, const char *key, long *out);

#endif /* JSON_H */
