#include "user_store.h"

#include <stdlib.h>
#include <string.h>

struct memory_store {
    struct user *users;
    size_t len;
    size_t cap;
};

static bool memory_save(void *ctx, const struct user *u)
{
    struct memory_store *m = ctx;

    for (size_t i = 0; i < m->len; i++) {
        if (strcmp(m->users[i].name, u->name) == 0) {
            m->users[i] = *u;
            return true;
        }
    }

    if (m->len == m->cap) {
        size_t cap = m->cap == 0 ? 4 : m->cap * 2;
        struct user *users = realloc(m->users, cap * sizeof *users);

        if (users == NULL) {
            return false;
        }
        m->users = users;
        m->cap = cap;
    }

    m->users[m->len] = *u;
    m->len++;
    return true;
}

static bool memory_get(void *ctx, const char *name, struct user *out)
{
    struct memory_store *m = ctx;

    for (size_t i = 0; i < m->len; i++) {
        if (strcmp(m->users[i].name, name) == 0) {
            if (out != NULL) {
                *out = m->users[i];
            }
            return true;
        }
    }

    return false;
}

struct user_store memory_store_new(void)
{
    struct memory_store *m = calloc(1, sizeof *m);

    return (struct user_store){
        .save = memory_save, .get = memory_get, .ctx = m};
}

void memory_store_free(struct user_store store)
{
    struct memory_store *m = store.ctx;

    if (m == NULL) {
        return;
    }
    free(m->users);
    free(m);
}
