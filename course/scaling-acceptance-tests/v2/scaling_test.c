#include <unistd.h>

#include "fixture.h"
#include "mcaccept.h"
#include "mctest.h"

static const char *server_path = "./echo_server";

static void test_round_trip_with_a_fixture(void)
{
    struct server_fixture f;

    CHECK_TRUE(fixture_start(&f, server_path));

    int fd = fixture_connect(&f);
    CHECK_TRUE(fd >= 0);

    if (fd >= 0) {
        char reply[64];

        accept_roundtrip(fd, "ping", reply, sizeof reply);
        CHECK_STR(reply, "ping");
        close(fd);
    }

    CHECK_TRUE(fixture_stop(&f));
}

static void test_many_fresh_servers(void)
{
    for (int i = 0; i < 25; i++) {
        struct server_fixture f;

        CHECK_TRUE(fixture_start(&f, server_path));

        int fd = fixture_connect(&f);
        CHECK_TRUE(fd >= 0);

        if (fd >= 0) {
            char reply[64];

            accept_roundtrip(fd, "hi", reply, sizeof reply);
            CHECK_STR(reply, "hi");
            close(fd);
        }

        CHECK_TRUE(fixture_stop(&f));
    }
}

int main(int argc, char **argv)
{
    if (argc > 1) {
        server_path = argv[1];
    }
    RUN_TEST(test_round_trip_with_a_fixture);
    RUN_TEST(test_many_fresh_servers);
    return test_summary();
}
