#include <string.h>

#include "handler.h"
#include "mctest.h"
#include "player_store.h"

static void request_get(const char *path, struct http_request *request)
{
    memset(request, 0, sizeof *request);
    strcpy(request->method, "GET");
    strcpy(request->path, path);
}

static void test_get_player_score(void)
{
    struct player_store store;
    struct http_request request;
    struct http_response response;

    player_store_init(&store);
    player_store_record_win(&store, "Pepper");
    player_store_record_win(&store, "Pepper");
    player_store_record_win(&store, "Pepper");

    request_get("/players/Pepper", &request);
    app_handler(&request, &response, &store);

    CHECK_INT(response.status, 200);
    CHECK_STR(response.body, "3");
    CHECK_STR(response.content_type, "text/plain");

    player_store_free(&store);
}

static void test_unknown_player_is_not_found(void)
{
    struct player_store store;
    struct http_request request;
    struct http_response response;

    player_store_init(&store);

    request_get("/players/Nobody", &request);
    app_handler(&request, &response, &store);

    CHECK_INT(response.status, 404);

    player_store_free(&store);
}

static void test_unknown_route_is_not_found(void)
{
    struct player_store store;
    struct http_request request;
    struct http_response response;

    player_store_init(&store);

    request_get("/nope", &request);
    app_handler(&request, &response, &store);

    CHECK_INT(response.status, 404);

    player_store_free(&store);
}

int main(void)
{
    RUN_TEST(test_get_player_score);
    RUN_TEST(test_unknown_player_is_not_found);
    RUN_TEST(test_unknown_route_is_not_found);
    return test_summary();
}
