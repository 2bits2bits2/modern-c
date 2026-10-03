#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "handler.h"
#include "mctest.h"
#include "persistence.h"

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

static void test_league_is_sorted(void)
{
    struct app_context app;
    struct http_request request;
    struct http_response response;

    memset(&app, 0, sizeof app);
    player_store_init(&app.store);

    player_store_record_win(&app.store, "Floyd");
    player_store_record_win(&app.store, "Pepper");
    player_store_record_win(&app.store, "Pepper");
    player_store_record_win(&app.store, "Pepper");

    make_request("GET", "/league", NULL, &request);
    app_handler(&request, &response, &app);

    CHECK_INT(response.status, 200);
    CHECK_STR(response.body, "[{\"name\":\"Pepper\",\"wins\":3},"
                             "{\"name\":\"Floyd\",\"wins\":1}]");

    player_store_free(&app.store);
}

static void test_post_persists(void)
{
    char path[] = "/tmp/mctest-io-handlerXXXXXX";
    struct app_context app;
    struct http_request request;
    struct http_response response;
    struct player_store reloaded;
    int wins = 0;
    int fd = mkstemp(path);

    CHECK_TRUE(fd >= 0);
    if (fd >= 0) {
        close(fd);
    }
    unlink(path);

    memset(&app, 0, sizeof app);
    player_store_init(&app.store);
    app.persist = true;
    snprintf(app.store_path, sizeof app.store_path, "%s", path);

    make_request("POST", "/players/Pepper", "{\"wins\":2}", &request);
    app_handler(&request, &response, &app);
    CHECK_INT(response.status, 201);

    player_store_init(&reloaded);
    CHECK_TRUE(player_store_load(&reloaded, path));
    CHECK_TRUE(player_store_get_wins(&reloaded, "Pepper", &wins));
    CHECK_INT(wins, 2);

    player_store_free(&reloaded);
    player_store_free(&app.store);
    unlink(path);
}

int main(void)
{
    RUN_TEST(test_league_is_sorted);
    RUN_TEST(test_post_persists);
    return test_summary();
}
