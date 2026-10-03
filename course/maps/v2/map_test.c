#include <stdio.h>

#include "map.h"
#include "mctest.h"

static void test_many_keys_and_resize(void)
{
    struct map *m = map_new();
    char key[32];

    for (int i = 0; i < 5000; i++) {
        snprintf(key, sizeof key, "key-%d", i);
        CHECK_TRUE(map_put(m, key, i));
    }

    CHECK_INT(m->len, 5000);
    CHECK_TRUE(m->nbuckets > 16);

    for (int i = 0; i < 5000; i++) {
        int got = -1;
        snprintf(key, sizeof key, "key-%d", i);
        CHECK_TRUE(map_get(m, key, &got));
        CHECK_INT(got, i);
    }

    map_free(m);
}

static void test_delete(void)
{
    struct map *m = map_new();
    int got = 0;

    CHECK_TRUE(map_put(m, "a", 1));
    CHECK_TRUE(map_delete(m, "a"));
    CHECK_TRUE(!map_get(m, "a", &got));
    CHECK_INT(m->len, 0);

    map_free(m);
}

static void test_delete_missing(void)
{
    struct map *m = map_new();

    CHECK_TRUE(!map_delete(m, "nope"));

    map_free(m);
}

int main(void)
{
    RUN_TEST(test_many_keys_and_resize);
    RUN_TEST(test_delete);
    RUN_TEST(test_delete_missing);
    return test_summary();
}
