#include "users.h"

#include <string.h>

void users_init(struct users *users)
{
    users->len = 0;
}

static bool contains(const struct users *users, const char *name)
{
    for (size_t i = 0; i < users->len; i++) {
        if (strcmp(users->names[i], name) == 0) {
            return true;
        }
    }
    return false;
}

bool users_register(struct users *users, const char *name,
                    struct app_error *err)
{
    error_clear(err);

    if (name[0] == '\0') {
        error_set(err, APP_ERR_INVALID, "name must not be empty");
        return false;
    }
    if (contains(users, name)) {
        error_set(err, APP_ERR_CONFLICT, "user %s already exists", name);
        return false;
    }
    if (users->len >= USERS_MAX) {
        error_set(err, APP_ERR_INTERNAL, "user limit reached");
        return false;
    }

    strncpy(users->names[users->len], name, USER_NAME_MAX - 1);
    users->names[users->len][USER_NAME_MAX - 1] = '\0';
    users->len++;
    return true;
}

bool users_find(const struct users *users, const char *name,
                struct app_error *err)
{
    error_clear(err);

    if (!contains(users, name)) {
        error_set(err, APP_ERR_NOT_FOUND, "no such user: %s", name);
        return false;
    }

    return true;
}
