#include <string.h>

#include "json.h"
#include "mctest.h"
#include "player_store.h"

static void test_encode_league(void)
{
    struct player_store store;
    char buf[256];

    player_store_init(&store);
    player_store_record_win(&store, "Pepper");
    player_store_record_win(&store, "Pepper");
    player_store_record_win(&store, "Floyd");

    json_encode_league(&store, buf, sizeof buf);

    CHECK_STR(buf, "[{\"name\":\"Pepper\",\"wins\":2},"
                   "{\"name\":\"Floyd\",\"wins\":1}]");

    player_store_free(&store);
}

static void test_encode_empty_league(void)
{
    struct player_store store;
    char buf[16];

    player_store_init(&store);

    json_encode_league(&store, buf, sizeof buf);
    CHECK_STR(buf, "[]");

    player_store_free(&store);
}

static void test_encode_escapes_quotes(void)
{
    struct player_store store;
    char buf[64];

    player_store_init(&store);
    player_store_record_win(&store, "a\"b");

    json_encode_league(&store, buf, sizeof buf);
    CHECK_STR(buf, "[{\"name\":\"a\\\"b\",\"wins\":1}]");

    player_store_free(&store);
}

static void test_parse_string(void)
{
    char out[32];

    CHECK_TRUE(
        json_get_string("{\"name\":\"Pepper\"}", "name", out, sizeof out));
    CHECK_STR(out, "Pepper");

    CHECK_TRUE(
        !json_get_string("{\"name\":\"Pepper\"}", "missing", out, sizeof out));
}

static void test_parse_int(void)
{
    long value = 0;

    CHECK_TRUE(
        json_get_int("{\"name\":\"Pepper\",\"wins\":10}", "wins", &value));
    CHECK_INT(value, 10);

    CHECK_TRUE(!json_get_int("{\"name\":\"Pepper\"}", "wins", &value));
}

int main(void)
{
    RUN_TEST(test_encode_league);
    RUN_TEST(test_encode_empty_league);
    RUN_TEST(test_encode_escapes_quotes);
    RUN_TEST(test_parse_string);
    RUN_TEST(test_parse_int);
    return test_summary();
}
