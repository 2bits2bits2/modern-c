#include "mctest.h"
#include "template.h"

static void test_multiple_bindings(void)
{
    char buf[64];
    const struct binding bindings[] = {
        {.key = "name", .value = "Chris"},
        {.key = "language", .value = "C"},
    };

    render_all("Hello, {name}! You are writing {language}.", bindings, 2, buf,
               sizeof buf);

    CHECK_STR(buf, "Hello, Chris! You are writing C.");
}

static void test_unknown_placeholder_is_left_alone(void)
{
    char buf[64];
    const struct binding bindings[] = {{.key = "name", .value = "Chris"}};

    render_all("{greeting}, {name}!", bindings, 1, buf, sizeof buf);

    CHECK_STR(buf, "{greeting}, Chris!");
}

int main(void)
{
    RUN_TEST(test_multiple_bindings);
    RUN_TEST(test_unknown_placeholder_is_left_alone);
    return test_summary();
}
