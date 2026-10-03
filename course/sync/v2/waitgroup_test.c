#include <pthread.h>

#include "mctest.h"
#include "waitgroup.h"

#define NUM_WORKERS 8

struct worker_args {
    struct waitgroup *wg;
    int *slot;
    int value;
};

static void *worker(void *arg)
{
    struct worker_args *a = arg;

    *a->slot = a->value;
    wg_done(a->wg);
    return NULL;
}

static void test_waitgroup(void)
{
    struct waitgroup wg;
    pthread_t threads[NUM_WORKERS];
    struct worker_args args[NUM_WORKERS];
    int slots[NUM_WORKERS] = {0};

    wg_init(&wg);
    wg_add(&wg, NUM_WORKERS);

    for (int i = 0; i < NUM_WORKERS; i++) {
        args[i] =
            (struct worker_args){.wg = &wg, .slot = &slots[i], .value = i + 1};
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }

    wg_wait(&wg);

    for (int i = 0; i < NUM_WORKERS; i++) {
        CHECK_INT(slots[i], i + 1);
    }

    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(threads[i], NULL);
    }

    wg_destroy(&wg);
}

int main(void)
{
    RUN_TEST(test_waitgroup);
    return test_summary();
}
