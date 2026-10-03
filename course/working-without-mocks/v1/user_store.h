#ifndef USER_STORE_H
#define USER_STORE_H

#include <stdbool.h>

#define USER_NAME_MAX 64

struct user {
    char name[USER_NAME_MAX];
    int age;
};

/*
 * A user store is any place we can save users to and read them back from.
 * The implementation is a pair of function pointers plus its own state.
 */
struct user_store {
    bool (*save)(void *ctx, const struct user *u);
    bool (*get)(void *ctx, const char *name, struct user *out);
    void *ctx;
};

struct user_store memory_store_new(void);
void memory_store_free(struct user_store store);

#endif /* USER_STORE_H */
