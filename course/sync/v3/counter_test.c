#include <pthread.h>

#include "counter.h"
#include "mctest.h"

#define NUM_THREADS 8
#define INCREMENTS 100000

static void *worker(void *arg)
{
    struct counter *c = arg;

    for (int i = 0; i < INCREMENTS; i++) {
        counter_inc(c);
    }

    return NULL;
}

static void test_concurrent_increments(void)
{
    struct counter c;
    pthread_t threads[NUM_THREADS];

    counter_init(&c);

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, worker, &c);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    CHECK_INT(counter_value(&c), NUM_THREADS * INCREMENTS);
}

int main(void)
{
    RUN_TEST(test_concurrent_increments);
    return test_summary();
}
