#include "mctest.h"
#include "website.h"

static bool fake_check(const char *url)
{
    return url[0] == 'h';
}

static void test_sequential(void)
{
    struct website sites[] = {
        {.url = "https://a", .ok = false},
        {.url = "ftp://b", .ok = false},
    };

    check_all_sequential(sites, 2, fake_check);

    CHECK_TRUE(sites[0].ok);
    CHECK_TRUE(!sites[1].ok);
}

static void test_concurrent(void)
{
    struct website sites[] = {
        {.url = "https://a", .ok = false},
        {.url = "https://b", .ok = false},
        {.url = "ftp://c", .ok = false},
    };

    check_all_concurrent(sites, 3, fake_check);

    CHECK_TRUE(sites[0].ok);
    CHECK_TRUE(sites[1].ok);
    CHECK_TRUE(!sites[2].ok);
}

int main(void)
{
    RUN_TEST(test_sequential);
    RUN_TEST(test_concurrent);
    return test_summary();
}
