#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "mcaccept.h"
#include "mctest.h"

static const char *server_path = "./server";

static void read_until(int fd, char *buf, size_t cap, const char *marker)
{
    size_t total = 0;

    while (total + 1 < cap && strstr(buf, marker) == NULL) {
        ssize_t n = read(fd, buf + total, cap - 1 - total);

        if (n <= 0) {
            break;
        }
        total += (size_t)n;
        buf[total] = '\0';
    }
}

static void read_exactly(int fd, unsigned char *buf, size_t n)
{
    size_t got = 0;

    while (got < n) {
        ssize_t m = read(fd, buf + got, n - got);

        if (m <= 0) {
            return;
        }
        got += (size_t)m;
    }
}

static void test_handshake_and_echo(void)
{
    char *argv[4] = {(char *)server_path, "--port", "0", NULL};
    int capture = -1;
    char response[1024] = {0};

    pid_t pid = accept_spawn_capture(server_path, argv, &capture);
    CHECK_TRUE(pid > 0);

    int port = accept_read_port(capture, 2000);
    close(capture);
    CHECK_TRUE(port > 0);

    int fd = accept_connect_tcp("127.0.0.1", port, 2000);
    CHECK_TRUE(fd >= 0);
    if (fd < 0) {
        accept_terminate(pid, 2000, NULL);
        return;
    }

    const char *handshake = "GET /ws HTTP/1.1\r\n"
                            "Host: localhost\r\n"
                            "Upgrade: websocket\r\n"
                            "Connection: Upgrade\r\n"
                            "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                            "Sec-WebSocket-Version: 13\r\n\r\n";
    CHECK_INT(write(fd, handshake, strlen(handshake)), (int)strlen(handshake));

    read_until(fd, response, sizeof response, "\r\n\r\n");
    CHECK_TRUE(strstr(response, "101 Switching Protocols") != NULL);
    CHECK_TRUE(strstr(response, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=") != NULL);

    /* Send a masked text frame "hi". */
    unsigned char mask[4] = {0x11, 0x22, 0x33, 0x44};
    unsigned char frame[16];
    const char *message = "hi";
    size_t n = 0;

    frame[n++] = 0x81;
    frame[n++] = 0x80 | (unsigned char)strlen(message);
    memcpy(frame + n, mask, 4);
    n += 4;
    for (size_t i = 0; i < strlen(message); i++) {
        frame[n++] = (unsigned char)message[i] ^ mask[i % 4];
    }
    CHECK_INT(write(fd, frame, n), (int)n);

    unsigned char echo[8];
    read_exactly(fd, echo, 4);
    CHECK_INT(echo[0], 0x81);
    CHECK_INT(echo[1], 0x02);
    CHECK_INT(echo[2], 'h');
    CHECK_INT(echo[3], 'i');

    /* Close handshake. */
    unsigned char close_frame[6] = {0x88, 0x80, 0, 0, 0, 0};
    (void)write(fd, close_frame, sizeof close_frame);
    close(fd);

    int status = 0;
    CHECK_TRUE(accept_terminate(pid, 2000, &status));
    CHECK_TRUE(WIFEXITED(status));
}

int main(int argc, char **argv)
{
    if (argc > 1) {
        server_path = argv[1];
    }
    RUN_TEST(test_handshake_and_echo);
    return test_summary();
}
