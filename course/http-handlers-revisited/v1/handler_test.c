#include <string.h>

#include "handler.h"
#include "mctest.h"

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

static void test_route_with_parameter(void)
{
    struct app_context app;
    struct http_request request;
    struct http_response response;

    memset(&app, 0, sizeof app);
    player_store_init(&app.store);
    player_store_record_win(&app.store, "Pepper");

    make_request("GET", "/players/Pepper", NULL, &request);
    app_handler(&request, &response, &app);

    CHECK_INT(response.status, 200);
    CHECK_STR(response.body, "{\"name\":\"Pepper\",\"wins\":1}");

    player_store_free(&app.store);
}

static void test_post_route(void)
{
    struct app_context app;
    struct http_request request;
    struct http_response response;
    int wins = 0;

    memset(&app, 0, sizeof app);
    player_store_init(&app.store);

    make_request("POST", "/players/Pepper", "{\"wins\":2}", &request);
    app_handler(&request, &response, &app);

    CHECK_INT(response.status, 201);
    CHECK_TRUE(player_store_get_wins(&app.store, "Pepper", &wins));
    CHECK_INT(wins, 2);

    player_store_free(&app.store);
}

static void test_league_route(void)
{
    struct app_context app;
    struct http_request request;
    struct http_response response;

    memset(&app, 0, sizeof app);
    player_store_init(&app.store);
    player_store_record_win(&app.store, "Pepper");

    make_request("GET", "/league", NULL, &request);
    app_handler(&request, &response, &app);

    CHECK_INT(response.status, 200);
    CHECK_STR(response.body, "[{\"name\":\"Pepper\",\"wins\":1}]");

    player_store_free(&app.store);
}

static void test_unknown_route_is_404(void)
{
    struct app_context app;
    struct http_request request;
    struct http_response response;

    memset(&app, 0, sizeof app);
    player_store_init(&app.store);

    make_request("GET", "/nope", NULL, &request);
    app_handler(&request, &response, &app);
    CHECK_INT(response.status, 404);

    player_store_free(&app.store);
}

static void test_wrong_method_is_405(void)
{
    struct app_context app;
    struct http_request request;
    struct http_response response;

    memset(&app, 0, sizeof app);
    player_store_init(&app.store);

    make_request("DELETE", "/players/Pepper", NULL, &request);
    app_handler(&request, &response, &app);
    CHECK_INT(response.status, 405);

    player_store_free(&app.store);
}

int main(void)
{
    RUN_TEST(test_route_with_parameter);
    RUN_TEST(test_post_route);
    RUN_TEST(test_league_route);
    RUN_TEST(test_unknown_route_is_404);
    RUN_TEST(test_wrong_method_is_405);
    return test_summary();
}
