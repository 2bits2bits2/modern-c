#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "mctest.h"
#include "reader.h"

static void test_copies_until_eof(void)
{
    int in[2];
    int out[2];
    struct read_context ctx;
    char buf[64];

    CHECK_INT(pipe(in), 0);
    CHECK_INT(pipe(out), 0);
    read_context_init(&ctx);

    CHECK_INT(write(in[1], "abc", 3), 3);
    CHECK_INT(write(in[1], "def", 3), 3);
    close(in[1]); /* EOF after the data */

    int n = copy_with_context(in[0], out[1], &ctx, 1000);
    CHECK_INT(n, 6);

    ssize_t got = read(out[0], buf, sizeof buf - 1);
    buf[got > 0 ? got : 0] = '\0';
    CHECK_STR(buf, "abcdef");

    close(in[0]);
    close(out[0]);
    close(out[1]);
}

struct cancel_args {
    struct read_context *ctx;
};

static void *cancel_soon(void *arg)
{
    struct cancel_args *args = arg;
    struct timespec pause = {.tv_sec = 0, .tv_nsec = 20 * 1000 * 1000};

    nanosleep(&pause, NULL);
    read_context_cancel(args->ctx);
    return NULL;
}

static void test_copy_is_interrupted_by_cancellation(void)
{
    int in[2];
    int out[2];
    struct read_context ctx;
    struct cancel_args args = {.ctx = &ctx};
    pthread_t thread;

    CHECK_INT(pipe(in), 0);
    CHECK_INT(pipe(out), 0);
    read_context_init(&ctx);

    /* Nothing will ever be written; only cancellation ends the copy. */
    pthread_create(&thread, NULL, cancel_soon, &args);

    int n = copy_with_context(in[0], out[1], &ctx, -1);
    CHECK_INT(n, -1);
    CHECK_TRUE(read_context_done(&ctx));

    pthread_join(thread, NULL);
    close(in[0]);
    close(in[1]);
    close(out[0]);
    close(out[1]);
}

int main(void)
{
    RUN_TEST(test_copies_until_eof);
    RUN_TEST(test_copy_is_interrupted_by_cancellation);
    return test_summary();
}
