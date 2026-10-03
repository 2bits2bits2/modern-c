#include <unistd.h>

#include "mctest.h"
#include "ready.h"

static void test_timeout(void)
{
    int a[2];
    int b[2];

    CHECK_INT(pipe(a), 0);
    CHECK_INT(pipe(b), 0);

    CHECK_INT(wait_ready(a[0], b[0], 20), READY_NONE);

    close(a[0]);
    close(a[1]);
    close(b[0]);
    close(b[1]);
}

static void test_first_is_ready(void)
{
    int a[2];
    int b[2];

    pipe(a);
    pipe(b);
    CHECK_INT(write(a[1], "x", 1), 1);

    CHECK_INT(wait_ready(a[0], b[0], 1000), READY_FIRST);

    close(a[0]);
    close(a[1]);
    close(b[0]);
    close(b[1]);
}

static void test_second_is_ready(void)
{
    int a[2];
    int b[2];

    pipe(a);
    pipe(b);
    CHECK_INT(write(b[1], "x", 1), 1);

    CHECK_INT(wait_ready(a[0], b[0], 1000), READY_SECOND);

    close(a[0]);
    close(a[1]);
    close(b[0]);
    close(b[1]);
}

int main(void)
{
    RUN_TEST(test_timeout);
    RUN_TEST(test_first_is_ready);
    RUN_TEST(test_second_is_ready);
    return test_summary();
}
