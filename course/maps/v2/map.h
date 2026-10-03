#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stddef.h>

struct map_entry;

struct map {
    struct map_entry **buckets;
    size_t nbuckets;
    size_t len;
};

struct map *map_new(void);
void map_free(struct map *m);
bool map_put(struct map *m, const char *key, int value);
bool map_get(const struct map *m, const char *key, int *out);
bool map_delete(struct map *m, const char *key);

#endif /* MAP_H */
