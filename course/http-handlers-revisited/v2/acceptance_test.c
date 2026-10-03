#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "mcaccept.h"
#include "mctest.h"

static const char *server_path = "./server";

static void test_routed_server(void)
{
    char seed[] = "Pepper=5";
    char *argv[6] = {(char *)server_path, "--port", "0", "--seed", seed, NULL};
    int capture = -1;
    char reply[1024];

    pid_t pid = accept_spawn_capture(server_path, argv, &capture);
    CHECK_TRUE(pid > 0);

    int port = accept_read_port(capture, 2000);
    close(capture);
    CHECK_TRUE(port > 0);

    int fd = accept_connect_tcp("127.0.0.1", port, 2000);
    CHECK_TRUE(fd >= 0);
    if (fd >= 0) {
        accept_roundtrip(fd,
                         "GET /players/Pepper HTTP/1.1\r\n"
                         "Host: localhost\r\n"
                         "Connection: close\r\n\r\n",
                         reply, sizeof reply);
        CHECK_TRUE(strstr(reply, "200 OK") != NULL);
        CHECK_TRUE(strstr(reply, "\"wins\":5") != NULL);
        close(fd);
    }

    fd = accept_connect_tcp("127.0.0.1", port, 2000);
    if (fd >= 0) {
        accept_roundtrip(fd,
                         "DELETE /players/Pepper HTTP/1.1\r\n"
                         "Host: localhost\r\n"
                         "Connection: close\r\n\r\n",
                         reply, sizeof reply);
        CHECK_TRUE(strstr(reply, "405 Method Not Allowed") != NULL);
        close(fd);
    }

    int status = 0;
    CHECK_TRUE(accept_terminate(pid, 2000, &status));
    CHECK_TRUE(WIFEXITED(status));
}

int main(int argc, char **argv)
{
    if (argc > 1) {
        server_path = argv[1];
    }
    RUN_TEST(test_routed_server);
    return test_summary();
}
