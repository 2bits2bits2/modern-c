#include <sys/wait.h>
#include <unistd.h>

#include "mcaccept.h"
#include "mctest.h"

static const char *server_path = "./server";

static void test_shuts_down_gracefully(void)
{
    char socket_path[128];
    char *argv[3];

    accept_socket_path(socket_path, sizeof socket_path, "accept-v2");
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

        accept_roundtrip(fd, "Chris", reply, sizeof reply);
        CHECK_STR(reply, "Hello, Chris!");
        close(fd);
    }

    int status = 0;
    CHECK_TRUE(accept_terminate(pid, 2000, &status));
    CHECK_TRUE(WIFEXITED(status));
    CHECK_INT(WEXITSTATUS(status), 0);

    unlink(socket_path);
}

int main(int argc, char **argv)
{
    if (argc > 1) {
        server_path = argv[1];
    }
    RUN_TEST(test_shuts_down_gracefully);
    return test_summary();
}
