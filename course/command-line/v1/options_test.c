#include "mctest.h"
#include "options.h"

static void test_defaults(void)
{
    char *argv[] = {"server", NULL};
    struct options options;

    CHECK_TRUE(parse_options(1, argv, &options));
    CHECK_INT(options.port, 0);
    CHECK_TRUE(!options.persist);
}

static void test_port_and_store(void)
{
    char *argv[] = {"server", "-p", "8080", "-s", "/tmp/store", NULL};
    struct options options;

    CHECK_TRUE(parse_options(5, argv, &options));
    CHECK_INT(options.port, 8080);
    CHECK_TRUE(options.persist);
    CHECK_STR(options.store_path, "/tmp/store");
}

static void test_unknown_option(void)
{
    char *argv[] = {"server", "-x", NULL};
    struct options options;

    CHECK_TRUE(!parse_options(2, argv, &options));
}

int main(void)
{
    RUN_TEST(test_defaults);
    RUN_TEST(test_port_and_store);
    RUN_TEST(test_unknown_option);
    return test_summary();
}
