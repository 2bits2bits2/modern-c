#include "hello.h"
#include "mctest.h"

static void test_hello_to_a_person(void)
{
    char buf[64];

    hello("Chris", "English", buf, sizeof buf);

    CHECK_STR(buf, "Hello, Chris");
}

static void test_empty_name_defaults_to_world(void)
{
    char buf[64];

    hello("", "English", buf, sizeof buf);

    CHECK_STR(buf, "Hello, World");
}

static void test_in_spanish(void)
{
    char buf[64];

    hello("Elodie", "Spanish", buf, sizeof buf);

    CHECK_STR(buf, "Hola, Elodie");
}

static void test_in_french(void)
{
    char buf[64];

    hello("Lauren", "French", buf, sizeof buf);

    CHECK_STR(buf, "Bonjour, Lauren");
}

static void test_unknown_language_falls_back_to_english(void)
{
    char buf[64];

    hello("Chris", "Klingon", buf, sizeof buf);

    CHECK_STR(buf, "Hello, Chris");
}

int main(void)
{
    RUN_TEST(test_hello_to_a_person);
    RUN_TEST(test_empty_name_defaults_to_world);
    RUN_TEST(test_in_spanish);
    RUN_TEST(test_in_french);
    RUN_TEST(test_unknown_language_falls_back_to_english);
    return test_summary();
}
