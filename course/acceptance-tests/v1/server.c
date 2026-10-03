#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "greeting.h"

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

static void handle(int conn)
{
    char name[128];
    char reply[256];
    ssize_t n = read(conn, name, sizeof name - 1);

    if (n < 0) {
        return;
    }

    name[n] = '\0';
    for (size_t i = 0; name[i] != '\0'; i++) {
        if (name[i] == '\n' || name[i] == '\r') {
            name[i] = '\0';
            break;
        }
    }

    greeting(name, reply, sizeof reply);
    (void)write(conn, reply, strlen(reply));
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <socket-path>\n", argv[0]);
        return 2;
    }

    int listener = listen_on(argv[1]);
    if (listener < 0) {
        return 1;
    }

    for (;;) {
        int conn = accept(listener, NULL, NULL);
        if (conn < 0) {
            break;
        }
        handle(conn);
        close(conn);
    }

    close(listener);
    unlink(argv[1]);
    return 0;
}
