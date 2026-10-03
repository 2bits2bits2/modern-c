#ifndef RUNNER_H
#define RUNNER_H

#include <stddef.h>

/*
 * A command runner is injectable, so business logic that shells out can be
 * tested without running anything.
 */
struct command_runner {
    int (*run)(void *ctx, char *const argv[], char *out, size_t cap);
    void *ctx;
};

#endif /* RUNNER_H */
