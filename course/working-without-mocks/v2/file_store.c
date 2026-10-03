#include "user_store.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct file_store {
    char path[256];
};

static void copy_str(char *dst, size_t cap, const char *src)
{
    size_t i = 0;

    for (; src[i] != '\0' && i + 1 < cap; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

static bool file_save(void *ctx, const struct user *u)
{
    struct file_store *fs = ctx;
    FILE *f = fopen(fs->path, "a");
    int written;

    if (f == NULL) {
        return false;
    }

    written = fprintf(f, "%s,%d\n", u->name, u->age);
    fclose(f);
    return written > 0;
}

static bool file_get(void *ctx, const char *name, struct user *out)
{
    struct file_store *fs = ctx;
    FILE *f = fopen(fs->path, "r");
    char line[256];
    struct user candidate;
    bool found = false;

    if (f == NULL) {
        return false;
    }

    while (fgets(line, sizeof line, f) != NULL) {
        char *comma = strchr(line, ',');

        if (comma == NULL) {
            continue;
        }
        *comma = '\0';

        if (strcmp(line, name) == 0) {
            memset(&candidate, 0, sizeof candidate);
            copy_str(candidate.name, sizeof candidate.name, line);
            candidate.age = atoi(comma + 1);
            found = true;
        }
    }

    fclose(f);

    if (found && out != NULL) {
        *out = candidate;
    }
    return found;
}

struct user_store file_store_new(const char *path)
{
    struct file_store *fs = calloc(1, sizeof *fs);

    copy_str(fs->path, sizeof fs->path, path);

    return (struct user_store){.save = file_save, .get = file_get, .ctx = fs};
}

void file_store_free(struct user_store store)
{
    free(store.ctx);
}
