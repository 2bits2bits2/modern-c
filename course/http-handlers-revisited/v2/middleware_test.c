#include <stdio.h>
#include <string.h>

#include "mctest.h"
#include "middleware.h"

struct spy_log {
    char method[16];
    char path[64];
    int status;
    int calls;
};

static void spy_log_fn(void *ctx, const char *method, const char *path,
                       int status)
{
    struct spy_log *spy = ctx;

    snprintf(spy->method, sizeof spy->method, "%s", method);
    snprintf(spy->path, sizeof spy->path, "%s", path);
    spy->status = status;
    spy->calls++;
}

static void ok_handler(const struct http_request *request,
                       struct http_response *response, void *ctx)
{
    (void)request;
    (void)ctx;
    http_response_json(response, 200, "{}");
}

static void test_logs_after_the_handler_runs(void)
{
    struct spy_log spy = {0};
    struct logged_handler logged = {
        .next = ok_handler,
        .next_ctx = NULL,
        .log = spy_log_fn,
        .log_ctx = &spy,
    };
    struct http_request request;
    struct http_response response;

    memset(&request, 0, sizeof request);
    strcpy(request.method, "GET");
    strcpy(request.path, "/players/Pepper");

    logged_handle(&request, &response, &logged);

    CHECK_INT(spy.calls, 1);
    CHECK_STR(spy.method, "GET");
    CHECK_STR(spy.path, "/players/Pepper");
    CHECK_INT(spy.status, 200);
}

int main(void)
{
    RUN_TEST(test_logs_after_the_handler_runs);
    return test_summary();
}
