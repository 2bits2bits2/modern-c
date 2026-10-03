#include "user.h"

#include "exec.h"

static void copy_until_newline(char *dst, size_t cap, const char *src)
{
    size_t i = 0;

    for (; src[i] != '\0' && src[i] != '\n' && src[i] != '\r' && i + 1 < cap;
         i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

int current_user(struct command_runner runner, char *name, size_t cap)
{
    char output[128];
    char *argv[] = {"whoami", NULL};

    if (runner.run(runner.ctx, argv, output, sizeof output) <= 0) {
        return -1;
    }

    copy_until_newline(name, cap, output);
    return 0;
}
