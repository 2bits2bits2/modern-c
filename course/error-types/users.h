#ifndef USERS_H
#define USERS_H

#include <stdbool.h>
#include <stddef.h>

#include "error.h"

#define USERS_MAX 16
#define USER_NAME_MAX 32

struct users {
    char names[USERS_MAX][USER_NAME_MAX];
    size_t len;
};

void users_init(struct users *users);

/* Register a new name. On failure *err is set with a code and message. */
bool users_register(struct users *users, const char *name,
                    struct app_error *err);

/* Look up a name. */
bool users_find(const struct users *users, const char *name,
                struct app_error *err);

#endif /* USERS_H */
