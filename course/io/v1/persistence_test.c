#include <stdlib.h>
#include <unistd.h>

#include "mctest.h"
#include "persistence.h"
#include "player_store.h"

static void test_save_and_load_round_trip(void)
{
    char path[] = "/tmp/mctest-ioXXXXXX";
    int fd = mkstemp(path);
    struct player_store saved;
    struct player_store loaded;
    int wins = 0;

    CHECK_TRUE(fd >= 0);
    if (fd >= 0) {
        close(fd);
    }

    player_store_init(&saved);
    player_store_record_win(&saved, "Pepper");
    player_store_record_win(&saved, "Pepper");
    player_store_record_win(&saved, "Floyd");

    CHECK_TRUE(player_store_save(&saved, path));

    player_store_init(&loaded);
    CHECK_TRUE(player_store_load(&loaded, path));

    CHECK_TRUE(player_store_get_wins(&loaded, "Pepper", &wins));
    CHECK_INT(wins, 2);
    CHECK_TRUE(player_store_get_wins(&loaded, "Floyd", &wins));
    CHECK_INT(wins, 1);

    player_store_free(&saved);
    player_store_free(&loaded);
    unlink(path);
}

static void test_load_missing_file_is_empty(void)
{
    struct player_store store;

    player_store_init(&store);
    CHECK_TRUE(player_store_load(&store, "/nonexistent/mctest-io"));
    CHECK_INT(store.len, 0);
    player_store_free(&store);
}

static void test_sort_by_wins(void)
{
    struct player_store store;

    player_store_init(&store);
    player_store_record_win(&store, "Floyd");
    player_store_record_win(&store, "Pepper");
    player_store_record_win(&store, "Pepper");
    player_store_record_win(&store, "Pepper");

    player_store_sort_by_wins(&store);

    CHECK_STR(store.players[0].name, "Pepper");
    CHECK_INT(store.players[0].wins, 3);
    CHECK_STR(store.players[1].name, "Floyd");
    CHECK_INT(store.players[1].wins, 1);

    player_store_free(&store);
}

int main(void)
{
    RUN_TEST(test_save_and_load_round_trip);
    RUN_TEST(test_load_missing_file_is_empty);
    RUN_TEST(test_sort_by_wins);
    return test_summary();
}
