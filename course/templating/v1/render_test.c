#include "mctest.h"
#include "render.h"

static void test_replace_one(void)
{
    char buf[64];

    render("Hello, {name}!", "name", "Chris", buf, sizeof buf);

    CHECK_STR(buf, "Hello, Chris!");
}

static void test_replace_every_occurrence(void)
{
    char buf[64];

    render("{x} and {x} again", "x", "42", buf, sizeof buf);

    CHECK_STR(buf, "42 and 42 again");
}

static void test_no_placeholder(void)
{
    char buf[64];

    render("no placeholders here", "x", "42", buf, sizeof buf);

    CHECK_STR(buf, "no placeholders here");
}

int main(void)
{
    RUN_TEST(test_replace_one);
    RUN_TEST(test_replace_every_occurrence);
    RUN_TEST(test_no_placeholder);
    return test_summary();
}
