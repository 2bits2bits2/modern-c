#include <stdio.h>

#include "map.h"
#include "mctest.h"

static void test_put_and_get(void)
{
    struct map *m = map_new();
    int got = 0;

    CHECK_TRUE(map_put(m, "a", 1));
    CHECK_TRUE(map_get(m, "a", &got));
    CHECK_INT(got, 1);

    map_free(m);
}

static void test_get_missing(void)
{
    struct map *m = map_new();
    int got = 0;

    CHECK_TRUE(!map_get(m, "nope", &got));

    map_free(m);
}

static void test_overwrite(void)
{
    struct map *m = map_new();
    int got = 0;

    CHECK_TRUE(map_put(m, "a", 1));
    CHECK_TRUE(map_put(m, "a", 2));
    CHECK_TRUE(map_get(m, "a", &got));
    CHECK_INT(got, 2);
    CHECK_INT(m->len, 1);

    map_free(m);
}

static void test_many_keys(void)
{
    struct map *m = map_new();
    char key[32];

    for (int i = 0; i < 500; i++) {
        snprintf(key, sizeof key, "key-%d", i);
        CHECK_TRUE(map_put(m, key, i));
    }

    for (int i = 0; i < 500; i++) {
        int got = -1;
        snprintf(key, sizeof key, "key-%d", i);
        CHECK_TRUE(map_get(m, key, &got));
        CHECK_INT(got, i);
    }

    map_free(m);
}

int main(void)
{
    RUN_TEST(test_put_and_get);
    RUN_TEST(test_get_missing);
    RUN_TEST(test_overwrite);
    RUN_TEST(test_many_keys);
    return test_summary();
}
