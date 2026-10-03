#ifndef USER_STORE_H
#define USER_STORE_H

#include <stdbool.h>

#define USER_NAME_MAX 64

struct user {
    char name[USER_NAME_MAX];
    int age;
};

struct user_store {
    bool (*save)(void *ctx, const struct user *u);
    bool (*get)(void *ctx, const char *name, struct user *out);
    void *ctx;
};

struct user_store memory_store_new(void);
void memory_store_free(struct user_store store);

struct user_store file_store_new(const char *path);
void file_store_free(struct user_store store);

#endif /* USER_STORE_H */
