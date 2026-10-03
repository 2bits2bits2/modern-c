#include <string.h>

#include "handler.h"
#include "mctest.h"
#include "player_store.h"

static void make_request(const char *method, const char *path, const char *body,
                         struct http_request *request)
{
    memset(request, 0, sizeof *request);
    strcpy(request->method, method);
    strcpy(request->path, path);
    if (body != NULL) {
        strcpy(request->body, body);
        request->body_len = strlen(body);
    }
}

static void test_get_player_as_json(void)
{
    struct player_store store;
    struct http_request request;
    struct http_response response;

    player_store_init(&store);
    player_store_record_win(&store, "Pepper");
    player_store_record_win(&store, "Pepper");
    player_store_record_win(&store, "Pepper");

    make_request("GET", "/players/Pepper", NULL, &request);
    app_handler(&request, &response, &store);

    CHECK_INT(response.status, 200);
    CHECK_STR(response.content_type, "application/json");
    CHECK_STR(response.body, "{\"name\":\"Pepper\",\"wins\":3}");

    player_store_free(&store);
}

static void test_get_league(void)
{
    struct player_store store;
    struct http_request request;
    struct http_response response;

    player_store_init(&store);
    player_store_record_win(&store, "Pepper");
    player_store_record_win(&store, "Floyd");

    make_request("GET", "/league", NULL, &request);
    app_handler(&request, &response, &store);

    CHECK_INT(response.status, 200);
    CHECK_STR(response.body, "[{\"name\":\"Pepper\",\"wins\":1},"
                             "{\"name\":\"Floyd\",\"wins\":1}]");

    player_store_free(&store);
}

static void test_post_records_wins(void)
{
    struct player_store store;
    struct http_request request;
    struct http_response response;
    int wins = 0;

    player_store_init(&store);

    make_request("POST", "/players/Pepper", "{\"wins\":2}", &request);
    app_handler(&request, &response, &store);
    CHECK_INT(response.status, 201);
    CHECK_TRUE(player_store_get_wins(&store, "Pepper", &wins));
    CHECK_INT(wins, 2);

    player_store_free(&store);
}

static void test_post_rejects_bad_body(void)
{
    struct player_store store;
    struct http_request request;
    struct http_response response;

    player_store_init(&store);

    make_request("POST", "/players/Pepper", "{\"wins\":\"lots\"}", &request);
    app_handler(&request, &response, &store);
    CHECK_INT(response.status, 400);

    player_store_free(&store);
}

static void test_unsupported_method(void)
{
    struct player_store store;
    struct http_request request;
    struct http_response response;

    player_store_init(&store);

    make_request("DELETE", "/players/Pepper", NULL, &request);
    app_handler(&request, &response, &store);
    CHECK_INT(response.status, 405);

    player_store_free(&store);
}

int main(void)
{
    RUN_TEST(test_get_player_as_json);
    RUN_TEST(test_get_league);
    RUN_TEST(test_post_records_wins);
    RUN_TEST(test_post_rejects_bad_body);
    RUN_TEST(test_unsupported_method);
    return test_summary();
}
