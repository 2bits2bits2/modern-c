#include "counter.h"

void counter_init(struct counter *c)
{
    pthread_mutex_init(&c->mu, NULL);
    c->value = 0;
}

void counter_destroy(struct counter *c)
{
    pthread_mutex_destroy(&c->mu);
}

void counter_inc(struct counter *c)
{
    pthread_mutex_lock(&c->mu);
    c->value++;
    pthread_mutex_unlock(&c->mu);
}

long counter_value(struct counter *c)
{
    pthread_mutex_lock(&c->mu);
    long v = c->value;
    pthread_mutex_unlock(&c->mu);
    return v;
}
