#include "buffer.h"
#include "countdown.h"
#include "mctest.h"

struct spy {
    int calls;
    int last_seconds;
};

static void spy_sleep(void *ctx, int seconds)
{
    struct spy *s = ctx;
    s->calls++;
    s->last_seconds = seconds;
}

static void test_countdown(void)
{
    struct buffer b;
    struct spy spy = {0};
    struct sleeper sleeper = {.sleep = spy_sleep, .ctx = &spy};

    buffer_init(&b);
    countdown(buffer_writer(&b), sleeper);

    CHECK_STR(b.data, "3\n2\n1\nGo!");
    CHECK_INT(spy.calls, 4);
    CHECK_INT(spy.last_seconds, 1);

    buffer_free(&b);
}

int main(void)
{
    RUN_TEST(test_countdown);
    return test_summary();
}
