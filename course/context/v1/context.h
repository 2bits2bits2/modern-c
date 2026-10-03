#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdatomic.h>
#include <stdbool.h>

/*
 * A context carries cancellation. Share a pointer to one with any number of
 * workers; when it is cancelled they should stop what they are doing.
 */
struct context {
    atomic_bool cancelled;
};

void context_init(struct context *c);
void context_cancel(struct context *c);
bool context_done(const struct context *c);

#endif /* CONTEXT_H */
