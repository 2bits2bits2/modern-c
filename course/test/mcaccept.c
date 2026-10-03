#include "mcaccept.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void sleep_ms(int ms)
{
    struct timespec ts = {.tv_sec = ms / 1000,
                          .tv_nsec = (long)(ms % 1000) * 1000000L};
    nanosleep(&ts, NULL);
}

void accept_socket_path(char *buf, size_t cap, const char *name)
{
    static int counter;

    counter++;
    snprintf(buf, cap, "/tmp/mctest-%s-%d-%d.sock", name, (int)getpid(),
             counter);
}

pid_t accept_spawn(const char *path, char *const argv[])
{
    pid_t pid = fork();

    if (pid < 0) {
        return -1;
    }
    if (pid == 0) {
        execv(path, argv);
        _exit(127);
    }

    return pid;
}

pid_t accept_spawn_capture(const char *path, char *const argv[],
                           int *capture_fd)
{
    int fds[2];
    pid_t pid;

    if (pipe(fds) != 0) {
        return -1;
    }

    pid = fork();
    if (pid < 0) {
        close(fds[0]);
        close(fds[1]);
        return -1;
    }
    if (pid == 0) {
        close(fds[0]);
        dup2(fds[1], STDOUT_FILENO);
        close(fds[1]);
        execv(path, argv);
        _exit(127);
    }

    close(fds[1]);
    *capture_fd = fds[0];
    return pid;
}

int accept_connect_unix(const char *path, int timeout_ms)
{
    struct sockaddr_un addr;
    size_t path_len = strlen(path);
    int waited = 0;

    if (path_len >= sizeof addr.sun_path) {
        return -1;
    }

    memset(&addr, 0, sizeof addr);
    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, path, path_len + 1);

    while (waited <= timeout_ms) {
        int fd = socket(AF_UNIX, SOCK_STREAM, 0);

        if (fd < 0) {
            return -1;
        }
        if (connect(fd, (struct sockaddr *)&addr, sizeof addr) == 0) {
            return fd;
        }
        close(fd);

        sleep_ms(5);
        waited += 5;
    }

    return -1;
}

int accept_connect_tcp(const char *host, int port, int timeout_ms)
{
    struct sockaddr_in addr;
    int waited = 0;

    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        return -1;
    }

    while (waited <= timeout_ms) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);

        if (fd < 0) {
            return -1;
        }
        if (connect(fd, (struct sockaddr *)&addr, sizeof addr) == 0) {
            return fd;
        }
        close(fd);

        sleep_ms(5);
        waited += 5;
    }

    return -1;
}

ssize_t accept_roundtrip(int fd, const char *request, char *reply, size_t cap)
{
    size_t req_len = strlen(request);
    size_t total = 0;

    if (write(fd, request, req_len) != (ssize_t)req_len) {
        return -1;
    }

    while (total + 1 < cap) {
        ssize_t n = read(fd, reply + total, cap - 1 - total);

        if (n < 0) {
            return -1;
        }
        if (n == 0) {
            break;
        }
        total += (size_t)n;
    }

    reply[total] = '\0';
    return (ssize_t)total;
}

int accept_read_line(int fd, char *buf, size_t cap, int timeout_ms)
{
    size_t len = 0;
    struct pollfd pfd = {.fd = fd, .events = POLLIN, .revents = 0};

    while (len + 1 < cap) {
        int ready = poll(&pfd, 1, timeout_ms);
        char c;

        if (ready <= 0) {
            break;
        }
        if (read(fd, &c, 1) != 1) {
            break;
        }
        if (c == '\n') {
            break;
        }
        buf[len] = c;
        len++;
    }

    buf[len] = '\0';
    return (int)len;
}

int accept_read_port(int fd, int timeout_ms)
{
    char line[128];

    if (accept_read_line(fd, line, sizeof line, timeout_ms) < 0) {
        return -1;
    }

    const char *equals = strchr(line, '=');
    return atoi(equals != NULL ? equals + 1 : line);
}

bool accept_terminate(pid_t pid, int timeout_ms, int *exit_status)
{
    int waited = 0;

    if (kill(pid, SIGTERM) != 0) {
        return false;
    }

    while (waited <= timeout_ms) {
        int status = 0;
        pid_t result = waitpid(pid, &status, WNOHANG);

        if (result == pid) {
            if (exit_status != NULL) {
                *exit_status = status;
            }
            return true;
        }
        if (result < 0) {
            return false;
        }

        sleep_ms(5);
        waited += 5;
    }

    kill(pid, SIGKILL);
    waitpid(pid, NULL, 0);
    return false;
}

bool accept_wait(pid_t pid, int timeout_ms, int *exit_status)
{
    int waited = 0;

    while (waited <= timeout_ms) {
        int status = 0;
        pid_t result = waitpid(pid, &status, WNOHANG);

        if (result == pid) {
            if (exit_status != NULL) {
                *exit_status = status;
            }
            return true;
        }
        if (result < 0) {
            return false;
        }

        sleep_ms(5);
        waited += 5;
    }

    kill(pid, SIGKILL);
    waitpid(pid, NULL, 0);
    return false;
}
