#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "mcaccept.h"
#include "mctest.h"

static const char *server_path = "./server";

static int start_and_read_port(const char *server_path, const char *store,
                               pid_t *pid_out)
{
    char *argv[6];
    char store_arg[256];
    int capture = -1;
    int n = 0;

    argv[n++] = (char *)server_path;
    argv[n++] = "--port";
    argv[n++] = "0";
    if (store != NULL) {
        argv[n++] = "--store";
        snprintf(store_arg, sizeof store_arg, "%s", store);
        argv[n++] = store_arg;
    }
    argv[n] = NULL;

    *pid_out = accept_spawn_capture(server_path, argv, &capture);
    if (*pid_out <= 0) {
        return -1;
    }

    int port = accept_read_port(capture, 2000);
    close(capture);
    return port;
}

static void test_data_survives_a_restart(void)
{
    char store[] = "/tmp/mctest-io-acceptXXXXXX";
    int fd = mkstemp(store);
    char reply[512];
    pid_t pid;
    int status = 0;
    int port;

    CHECK_TRUE(fd >= 0);
    if (fd >= 0) {
        close(fd);
    }
    unlink(store);

    port = start_and_read_port(server_path, store, &pid);
    CHECK_TRUE(port > 0);

    int conn = accept_connect_tcp("127.0.0.1", port, 2000);
    CHECK_TRUE(conn >= 0);
    if (conn >= 0) {
        accept_roundtrip(conn,
                         "POST /players/Pepper HTTP/1.1\r\n"
                         "Host: localhost\r\n"
                         "Content-Length: 10\r\n"
                         "Connection: close\r\n\r\n"
                         "{\"wins\":3}",
                         reply, sizeof reply);
        CHECK_TRUE(strstr(reply, "201 Created") != NULL);
        close(conn);
    }
    CHECK_TRUE(accept_terminate(pid, 2000, &status));

    /* Restart with the same store and the data should still be there. */
    port = start_and_read_port(server_path, store, &pid);
    CHECK_TRUE(port > 0);

    conn = accept_connect_tcp("127.0.0.1", port, 2000);
    CHECK_TRUE(conn >= 0);
    if (conn >= 0) {
        accept_roundtrip(conn,
                         "GET /players/Pepper HTTP/1.1\r\n"
                         "Host: localhost\r\n"
                         "Connection: close\r\n\r\n",
                         reply, sizeof reply);
        CHECK_TRUE(strstr(reply, "200 OK") != NULL);
        CHECK_TRUE(strstr(reply, "\"wins\":3") != NULL);
        close(conn);
    }
    CHECK_TRUE(accept_terminate(pid, 2000, &status));

    unlink(store);
}

int main(int argc, char **argv)
{
    if (argc > 1) {
        server_path = argv[1];
    }
    RUN_TEST(test_data_survives_a_restart);
    return test_summary();
}
