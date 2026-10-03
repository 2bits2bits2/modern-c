#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "mcaccept.h"
#include "mctest.h"

#ifndef CLI_PATH
#define CLI_PATH "./cli"
#endif

static const char *server_path = "./server";

static void test_cli_records_a_win(void)
{
    char *server_argv[4] = {(char *)server_path, "-p", "0", NULL};
    int capture = -1;
    char reply[512];

    pid_t pid = accept_spawn_capture(server_path, server_argv, &capture);
    CHECK_TRUE(pid > 0);

    int port = accept_read_port(capture, 2000);
    close(capture);
    CHECK_TRUE(port > 0);

    char port_string[16];
    snprintf(port_string, sizeof port_string, "%d", port);
    char *cli_argv[6] = {(char *)CLI_PATH, "-p", port_string,
                         "Pepper",         "2",  NULL};

    pid_t cli = accept_spawn(CLI_PATH, cli_argv);
    CHECK_TRUE(cli > 0);

    int cli_status = 0;
    CHECK_TRUE(accept_wait(cli, 3000, &cli_status));
    CHECK_TRUE(WIFEXITED(cli_status));
    CHECK_INT(WEXITSTATUS(cli_status), 0);

    int fd = accept_connect_tcp("127.0.0.1", port, 2000);
    CHECK_TRUE(fd >= 0);
    if (fd >= 0) {
        accept_roundtrip(fd,
                         "GET /players/Pepper HTTP/1.1\r\n"
                         "Host: localhost\r\n"
                         "Connection: close\r\n\r\n",
                         reply, sizeof reply);
        CHECK_TRUE(strstr(reply, "\"wins\":2") != NULL);
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
    RUN_TEST(test_cli_records_a_win);
    return test_summary();
}
