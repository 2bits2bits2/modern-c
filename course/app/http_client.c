#include "http_client.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int http_post_wins(const char *host, int port, const char *name, int wins,
                   char *reply, size_t cap)
{
    struct sockaddr_in addr;
    char body[128];
    char request[512];
    size_t total = 0;
    int fd;

    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        return -1;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }
    if (connect(fd, (struct sockaddr *)&addr, sizeof addr) != 0) {
        close(fd);
        return -1;
    }

    snprintf(body, sizeof body, "{\"wins\":%d}", wins);
    snprintf(request, sizeof request,
             "POST /players/%s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             name, host, strlen(body), body);

    if (write(fd, request, strlen(request)) < 0) {
        close(fd);
        return -1;
    }

    while (total + 1 < cap) {
        ssize_t n = read(fd, reply + total, cap - 1 - total);

        if (n < 0) {
            close(fd);
            return -1;
        }
        if (n == 0) {
            break;
        }
        total += (size_t)n;
    }

    reply[total] = '\0';
    close(fd);
    return (int)total;
}
