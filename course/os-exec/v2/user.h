#ifndef USER_H
#define USER_H

#include <stddef.h>

#include "runner.h"

/*
 * Ask the operating system for the current user's name. The command runner is
 * injected so this can be tested with a fake.
 */
int current_user(struct command_runner runner, char *name, size_t cap);

#endif /* USER_H */
