#include <sys/wait.h>
#include <unistd.h>

#include "mcaccept.h"
#include "mctest.h"

static const char *server_path = "./echo_server";

static void test_round_trip(void)
{
    char socket_path[128];
    char delay[] = "0";

    accept_socket_path(socket_path, sizeof socket_path, "scaling-v1");
    unlink(socket_path);

    char *argv[4] = {(char *)server_path, socket_path, delay, NULL};
    pid_t pid = accept_spawn(server_path, argv);
    CHECK_TRUE(pid > 0);

    int fd = accept_connect_unix(socket_path, 2000);
    CHECK_TRUE(fd >= 0);

    if (fd >= 0) {
        char reply[64];

        accept_roundtrip(fd, "ping", reply, sizeof reply);
        CHECK_STR(reply, "ping");
        close(fd);
    }

    int status = 0;
    CHECK_TRUE(accept_terminate(pid, 2000, &status));
    unlink(socket_path);
}

static void test_slow_startup_is_waited_for(void)
{
    char socket_path[128];
    char delay[] = "300";

    accept_socket_path(socket_path, sizeof socket_path, "scaling-slow");
    unlink(socket_path);

    char *argv[4] = {(char *)server_path, socket_path, delay, NULL};
    pid_t pid = accept_spawn(server_path, argv);
    CHECK_TRUE(pid > 0);

    /* The server will not be listening for ~300ms. A fixed short sleep would
     * be flaky here; polling for readiness is not. */
    int fd = accept_connect_unix(socket_path, 2000);
    CHECK_TRUE(fd >= 0);

    if (fd >= 0) {
        close(fd);
    }

    int status = 0;
    CHECK_TRUE(accept_terminate(pid, 2000, &status));
    unlink(socket_path);
}

int main(int argc, char **argv)
{
    if (argc > 1) {
        server_path = argv[1];
    }
    RUN_TEST(test_round_trip);
    RUN_TEST(test_slow_startup_is_waited_for);
    return test_summary();
}
