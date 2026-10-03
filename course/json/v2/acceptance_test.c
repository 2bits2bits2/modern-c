#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "mcaccept.h"
#include "mctest.h"

static const char *server_path = "./server";

static void test_record_then_read_as_json(void)
{
    char *argv[4] = {(char *)server_path, "--port", "0", NULL};
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
                         "POST /players/Pepper HTTP/1.1\r\n"
                         "Host: localhost\r\n"
                         "Content-Length: 10\r\n"
                         "Connection: close\r\n\r\n"
                         "{\"wins\":3}",
                         reply, sizeof reply);
        CHECK_TRUE(strstr(reply, "201 Created") != NULL);
        close(fd);
    }

    fd = accept_connect_tcp("127.0.0.1", port, 2000);
    if (fd >= 0) {
        accept_roundtrip(fd,
                         "GET /players/Pepper HTTP/1.1\r\n"
                         "Host: localhost\r\n"
                         "Connection: close\r\n\r\n",
                         reply, sizeof reply);
        CHECK_TRUE(strstr(reply, "200 OK") != NULL);
        CHECK_TRUE(strstr(reply, "\"wins\":3") != NULL);
        close(fd);
    }

    fd = accept_connect_tcp("127.0.0.1", port, 2000);
    if (fd >= 0) {
        accept_roundtrip(fd,
                         "GET /league HTTP/1.1\r\n"
                         "Host: localhost\r\n"
                         "Connection: close\r\n\r\n",
                         reply, sizeof reply);
        CHECK_TRUE(strstr(reply, "\"name\":\"Pepper\"") != NULL);
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
    RUN_TEST(test_record_then_read_as_json);
    return test_summary();
}
