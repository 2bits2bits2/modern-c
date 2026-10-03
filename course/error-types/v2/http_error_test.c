#include "http_error.h"
#include "mctest.h"

static void test_error_becomes_a_status_and_json(void)
{
    struct app_error err;
    struct http_response response;

    error_set(&err, APP_ERR_NOT_FOUND, "no such user: nobody");
    error_write_response(&err, &response);

    CHECK_INT(response.status, 404);
    CHECK_STR(response.content_type, "application/json");
    CHECK_STR(response.body, "{\"error\":\"no such user: nobody\"}");
}

int main(void)
{
    RUN_TEST(test_error_becomes_a_status_and_json);
    return test_summary();
}
