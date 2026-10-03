#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t running = 1;

static void on_signal(int signal_number)
{
    (void)signal_number;
    running = 0;
}

static int listen_on(const char *path)
{
    struct sockaddr_un addr;
    size_t path_len = strlen(path);
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (fd < 0 || path_len >= sizeof addr.sun_path) {
        if (fd >= 0) {
            close(fd);
        }
        return -1;
    }

    memset(&addr, 0, sizeof addr);
    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, path, path_len + 1);

    unlink(path);
    if (bind(fd, (struct sockaddr *)&addr, sizeof addr) != 0 ||
        listen(fd, 16) != 0) {
        close(fd);
        return -1;
    }

    return fd;
}

static void echo(int conn)
{
    char buf[256];
    ssize_t n = read(conn, buf, sizeof buf);

    if (n > 0) {
        (void)write(conn, buf, (size_t)n);
    }
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "usage: %s <socket-path> <startup-delay-ms>\n",
                argv[0]);
        return 2;
    }

    int delay_ms = atoi(argv[2]);
    if (delay_ms > 0) {
        struct timespec ts = {.tv_sec = delay_ms / 1000,
                              .tv_nsec = (long)(delay_ms % 1000) * 1000000L};
        nanosleep(&ts, NULL);
    }

    struct sigaction action;
    memset(&action, 0, sizeof action);
    action.sa_handler = on_signal;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    int listener = listen_on(argv[1]);
    if (listener < 0) {
        return 1;
    }

    while (running) {
        int conn = accept(listener, NULL, NULL);

        if (conn < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }

        echo(conn);
        close(conn);
    }

    close(listener);
    unlink(argv[1]);
    return 0;
}
