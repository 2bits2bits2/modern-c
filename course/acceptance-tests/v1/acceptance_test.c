#include <sys/wait.h>
#include <unistd.h>

#include "mcaccept.h"
#include "mctest.h"

static const char *server_path = "./server";

static void test_greets_over_a_unix_socket(void)
{
    char socket_path[128];
    char *argv[3];
    int reply_len = 0;

    accept_socket_path(socket_path, sizeof socket_path, "accept-v1");
    unlink(socket_path);

    argv[0] = (char *)server_path;
    argv[1] = socket_path;
    argv[2] = NULL;

    pid_t pid = accept_spawn(server_path, argv);
    CHECK_TRUE(pid > 0);

    int fd = accept_connect_unix(socket_path, 2000);
    CHECK_TRUE(fd >= 0);

    if (fd >= 0) {
        char reply[128];

        reply_len = (int)accept_roundtrip(fd, "Chris", reply, sizeof reply);
        CHECK_TRUE(reply_len > 0);
        CHECK_STR(reply, "Hello, Chris!");
        close(fd);
    }

    int status = 0;
    CHECK_TRUE(accept_terminate(pid, 2000, &status));
    CHECK_TRUE(WIFSIGNALED(status));

    unlink(socket_path);
}

int main(int argc, char **argv)
{
    if (argc > 1) {
        server_path = argv[1];
    }
    RUN_TEST(test_greets_over_a_unix_socket);
    return test_summary();
}
