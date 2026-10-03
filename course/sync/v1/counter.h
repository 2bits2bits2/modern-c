#ifndef COUNTER_H
#define COUNTER_H

#include <pthread.h>

struct counter {
    pthread_mutex_t mu;
    long value;
};

void counter_init(struct counter *c);
void counter_destroy(struct counter *c);
void counter_inc(struct counter *c);
long counter_value(struct counter *c);

#endif /* COUNTER_H */
