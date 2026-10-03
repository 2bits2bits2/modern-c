#include <pthread.h>
#include <time.h>

#include "context.h"
#include "mctest.h"

struct worker {
    const struct context *ctx;
    long iterations;
};

static void *run_worker(void *arg)
{
    struct worker *w = arg;

    while (!context_done(w->ctx)) {
        w->iterations++;
    }

    return NULL;
}

static void test_a_new_context_is_not_done(void)
{
    struct context c;

    context_init(&c);

    CHECK_TRUE(!context_done(&c));
}

static void test_cancel_marks_it_done(void)
{
    struct context c;

    context_init(&c);
    context_cancel(&c);

    CHECK_TRUE(context_done(&c));
}

static void test_worker_stops_when_cancelled(void)
{
    struct context c;
    struct worker w = {.ctx = &c, .iterations = 0};
    pthread_t thread;
    struct timespec pause = {.tv_sec = 0, .tv_nsec = 2 * 1000 * 1000};

    context_init(&c);
    pthread_create(&thread, NULL, run_worker, &w);

    nanosleep(&pause, NULL);
    context_cancel(&c);
    pthread_join(thread, NULL);

    CHECK_TRUE(w.iterations > 0);
    CHECK_TRUE(context_done(&c));
}

int main(void)
{
    RUN_TEST(test_a_new_context_is_not_done);
    RUN_TEST(test_cancel_marks_it_done);
    RUN_TEST(test_worker_stops_when_cancelled);
    return test_summary();
}
