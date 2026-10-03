#include "counter.h"

void counter_init(struct counter *c)
{
    atomic_init(&c->value, 0);
}

void counter_inc(struct counter *c)
{
    atomic_fetch_add(&c->value, 1);
}

long counter_value(struct counter *c)
{
    return atomic_load(&c->value);
}
