#include <string.h>
#include <unistd.h>

#include "mctest.h"
#include "reader.h"

static void test_reads_available_data(void)
{
    int fds[2];
    struct read_context ctx;
    char buf[32];

    CHECK_INT(pipe(fds), 0);
    read_context_init(&ctx);
    CHECK_INT(write(fds[1], "hello", 5), 5);

    int n = read_with_context(fds[0], &ctx, buf, sizeof buf, 1000);
    CHECK_INT(n, 5);
    buf[n] = '\0';
    CHECK_STR(buf, "hello");

    close(fds[0]);
    close(fds[1]);
}

static void test_times_out_when_no_data(void)
{
    int fds[2];
    struct read_context ctx;
    char buf[32];

    CHECK_INT(pipe(fds), 0);
    read_context_init(&ctx);

    CHECK_INT(read_with_context(fds[0], &ctx, buf, sizeof buf, 30), 0);

    close(fds[0]);
    close(fds[1]);
}

static void test_reports_cancellation(void)
{
    int fds[2];
    struct read_context ctx;
    char buf[32];

    CHECK_INT(pipe(fds), 0);
    read_context_init(&ctx);
    read_context_cancel(&ctx);

    CHECK_INT(read_with_context(fds[0], &ctx, buf, sizeof buf, 1000), -1);

    close(fds[0]);
    close(fds[1]);
}

int main(void)
{
    RUN_TEST(test_reads_available_data);
    RUN_TEST(test_times_out_when_no_data);
    RUN_TEST(test_reports_cancellation);
    return test_summary();
}
