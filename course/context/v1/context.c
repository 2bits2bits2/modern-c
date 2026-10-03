#include "context.h"

void context_init(struct context *c)
{
    atomic_init(&c->cancelled, false);
}

void context_cancel(struct context *c)
{
    atomic_store(&c->cancelled, true);
}

bool context_done(const struct context *c)
{
    return atomic_load(&c->cancelled);
}
