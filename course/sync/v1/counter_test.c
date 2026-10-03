#include <pthread.h>

#include "counter.h"
#include "mctest.h"

#define NUM_THREADS 8
#define INCREMENTS 100000

struct worker_args {
    struct counter *counter;
    int times;
};

static void *worker(void *arg)
{
    struct worker_args *a = arg;

    for (int i = 0; i < a->times; i++) {
        counter_inc(a->counter);
    }

    return NULL;
}

static void test_concurrent_increments(void)
{
    struct counter c;
    pthread_t threads[NUM_THREADS];
    struct worker_args args[NUM_THREADS];

    counter_init(&c);

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i] = (struct worker_args){.counter = &c, .times = INCREMENTS};
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    CHECK_INT(counter_value(&c), NUM_THREADS * INCREMENTS);

    counter_destroy(&c);
}

int main(void)
{
    RUN_TEST(test_concurrent_increments);
    return test_summary();
}
