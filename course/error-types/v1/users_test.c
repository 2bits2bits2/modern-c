#include "mctest.h"
#include "users.h"

static void test_register_and_find(void)
{
    struct users users;
    struct app_error err;

    users_init(&users);

    CHECK_TRUE(users_register(&users, "chris", &err));
    CHECK_TRUE(!error_is_set(&err));
    CHECK_TRUE(users_find(&users, "chris", &err));
    CHECK_TRUE(!error_is_set(&err));
}

static void test_empty_name_is_invalid(void)
{
    struct users users;
    struct app_error err;

    users_init(&users);

    CHECK_TRUE(!users_register(&users, "", &err));
    CHECK_INT(err.code, APP_ERR_INVALID);
    CHECK_STR(err.message, "name must not be empty");
}

static void test_duplicate_is_a_conflict(void)
{
    struct users users;
    struct app_error err;

    users_init(&users);
    users_register(&users, "chris", &err);

    CHECK_TRUE(!users_register(&users, "chris", &err));
    CHECK_INT(err.code, APP_ERR_CONFLICT);
    CHECK_STR(err.message, "user chris already exists");
}

static void test_missing_user_includes_the_name(void)
{
    struct users users;
    struct app_error err;

    users_init(&users);

    CHECK_TRUE(!users_find(&users, "nobody", &err));
    CHECK_INT(err.code, APP_ERR_NOT_FOUND);
    CHECK_STR(err.message, "no such user: nobody");
}

static void test_codes_map_to_http_status(void)
{
    CHECK_INT(error_http_status(APP_ERR_INVALID), 400);
    CHECK_INT(error_http_status(APP_ERR_NOT_FOUND), 404);
    CHECK_INT(error_http_status(APP_ERR_CONFLICT), 409);
    CHECK_INT(error_http_status(APP_ERR_INTERNAL), 500);
    CHECK_INT(error_http_status(9999), 500);
}

int main(void)
{
    RUN_TEST(test_register_and_find);
    RUN_TEST(test_empty_name_is_invalid);
    RUN_TEST(test_duplicate_is_a_conflict);
    RUN_TEST(test_missing_user_includes_the_name);
    RUN_TEST(test_codes_map_to_http_status);
    return test_summary();
}
