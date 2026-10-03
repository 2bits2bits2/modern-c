#ifndef COUNTER_H
#define COUNTER_H

#include <stdatomic.h>

struct counter {
    atomic_long value;
};

void counter_init(struct counter *c);
void counter_inc(struct counter *c);
long counter_value(struct counter *c);

#endif /* COUNTER_H */
