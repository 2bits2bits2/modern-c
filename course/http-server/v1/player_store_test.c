#include <stdio.h>

#include "mctest.h"
#include "player_store.h"

static void test_record_and_get(void)
{
    struct player_store store;
    int wins = 0;

    player_store_init(&store);

    CHECK_TRUE(!player_store_get_wins(&store, "Pepper", &wins));

    player_store_record_win(&store, "Pepper");
    player_store_record_win(&store, "Pepper");
    player_store_record_win(&store, "Floyd");

    CHECK_TRUE(player_store_get_wins(&store, "Pepper", &wins));
    CHECK_INT(wins, 2);
    CHECK_TRUE(player_store_get_wins(&store, "Floyd", &wins));
    CHECK_INT(wins, 1);
    CHECK_TRUE(!player_store_get_wins(&store, "Nobody", &wins));

    player_store_free(&store);
}

static void test_many_players_grow(void)
{
    struct player_store store;
    char name[32];
    int wins = 0;

    player_store_init(&store);

    for (int i = 0; i < 100; i++) {
        snprintf(name, sizeof name, "player-%d", i);
        player_store_record_win(&store, name);
    }
    CHECK_INT(store.len, 100);

    for (int i = 0; i < 100; i++) {
        snprintf(name, sizeof name, "player-%d", i);
        CHECK_TRUE(player_store_get_wins(&store, name, &wins));
        CHECK_INT(wins, 1);
    }

    player_store_free(&store);
}

int main(void)
{
    RUN_TEST(test_record_and_get);
    RUN_TEST(test_many_players_grow);
    return test_summary();
}
