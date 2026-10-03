#include "map.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct map_entry {
    char *key;
    int value;
    struct map_entry *next;
};

#define INITIAL_BUCKETS 16

static size_t hash_string(const char *s)
{
    uint64_t h = 1469598103934665603ULL;

    for (const unsigned char *p = (const unsigned char *)s; *p != '\0'; p++) {
        h ^= *p;
        h *= 1099511628211ULL;
    }

    return (size_t)h;
}

static void free_buckets(struct map_entry **buckets, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        struct map_entry *e = buckets[i];
        while (e != NULL) {
            struct map_entry *next = e->next;
            free(e->key);
            free(e);
            e = next;
        }
    }
    free(buckets);
}

struct map *map_new(void)
{
    struct map *m = malloc(sizeof *m);

    if (m == NULL) {
        return NULL;
    }

    m->nbuckets = INITIAL_BUCKETS;
    m->len = 0;
    m->buckets = calloc(m->nbuckets, sizeof *m->buckets);

    if (m->buckets == NULL) {
        free(m);
        return NULL;
    }

    return m;
}

void map_free(struct map *m)
{
    if (m == NULL) {
        return;
    }

    free_buckets(m->buckets, m->nbuckets);
    free(m);
}

/* Grow the bucket array and re-home every entry. */
static bool map_resize(struct map *m, size_t new_nbuckets)
{
    struct map_entry **new_buckets = calloc(new_nbuckets, sizeof *new_buckets);

    if (new_buckets == NULL) {
        return false;
    }

    for (size_t i = 0; i < m->nbuckets; i++) {
        struct map_entry *e = m->buckets[i];
        while (e != NULL) {
            struct map_entry *next = e->next;
            size_t idx = hash_string(e->key) % new_nbuckets;
            e->next = new_buckets[idx];
            new_buckets[idx] = e;
            e = next;
        }
    }

    free(m->buckets);
    m->buckets = new_buckets;
    m->nbuckets = new_nbuckets;
    return true;
}

bool map_put(struct map *m, const char *key, int value)
{
    size_t idx = hash_string(key) % m->nbuckets;

    for (struct map_entry *e = m->buckets[idx]; e != NULL; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            e->value = value;
            return true;
        }
    }

    struct map_entry *e = malloc(sizeof *e);
    if (e == NULL) {
        return false;
    }

    e->key = strdup(key);
    if (e->key == NULL) {
        free(e);
        return false;
    }

    e->value = value;
    e->next = m->buckets[idx];
    m->buckets[idx] = e;
    m->len++;

    if (m->len > m->nbuckets * 3) {
        map_resize(m, m->nbuckets * 2);
    }

    return true;
}

bool map_get(const struct map *m, const char *key, int *out)
{
    size_t idx = hash_string(key) % m->nbuckets;

    for (const struct map_entry *e = m->buckets[idx]; e != NULL; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            if (out != NULL) {
                *out = e->value;
            }
            return true;
        }
    }

    return false;
}

bool map_delete(struct map *m, const char *key)
{
    size_t idx = hash_string(key) % m->nbuckets;
    struct map_entry **link = &m->buckets[idx];

    while (*link != NULL) {
        struct map_entry *e = *link;
        if (strcmp(e->key, key) == 0) {
            *link = e->next;
            free(e->key);
            free(e);
            m->len--;
            return true;
        }
        link = &e->next;
    }

    return false;
}
