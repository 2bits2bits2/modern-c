#include "http.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void http_response_init(struct http_response *res, int status,
                        const char *content_type)
{
    res->status = status;
    res->body_len = 0;
    res->body[0] = '\0';

    size_t len = strlen(content_type);
    if (len >= sizeof res->content_type) {
        len = sizeof res->content_type - 1;
    }
    memcpy(res->content_type, content_type, len);
    res->content_type[len] = '\0';
}

static void copy_body(struct http_response *res, const char *text)
{
    size_t len = strlen(text);

    if (len >= sizeof res->body) {
        len = sizeof res->body - 1;
    }
    memcpy(res->body, text, len);
    res->body[len] = '\0';
    res->body_len = len;
}

void http_response_text(struct http_response *res, int status, const char *text)
{
    http_response_init(res, status, "text/plain");
    copy_body(res, text);
}

void http_response_json(struct http_response *res, int status, const char *json)
{
    http_response_init(res, status, "application/json");
    copy_body(res, json);
}

bool http_parse_request(const char *raw, size_t len, struct http_request *out)
{
    const char *line_end = strstr(raw, "\r\n");
    const char *space1;
    const char *space2;
    const char *body;

    memset(out, 0, sizeof *out);

    if (line_end == NULL) {
        return false;
    }

    space1 = memchr(raw, ' ', (size_t)(line_end - raw));
    if (space1 == NULL || space1 == raw) {
        return false;
    }

    size_t method_len = (size_t)(space1 - raw);
    if (method_len >= sizeof out->method) {
        return false;
    }
    memcpy(out->method, raw, method_len);
    out->method[method_len] = '\0';

    const char *path_start = space1 + 1;
    space2 = memchr(path_start, ' ', (size_t)(line_end - path_start));
    size_t path_len = (space2 != NULL) ? (size_t)(space2 - path_start)
                                       : (size_t)(line_end - path_start);
    if (path_len == 0 || path_len >= sizeof out->path) {
        return false;
    }
    memcpy(out->path, path_start, path_len);
    out->path[path_len] = '\0';

    body = strstr(raw, "\r\n\r\n");
    if (body != NULL) {
        body += 4;
        size_t body_len = len - (size_t)(body - raw);
        if (body_len >= sizeof out->body) {
            body_len = sizeof out->body - 1;
        }
        memcpy(out->body, body, body_len);
        out->body[body_len] = '\0';
        out->body_len = body_len;
    }

    return true;
}

static const char *status_text(int status)
{
    switch (status) {
    case 200:
        return "OK";
    case 201:
        return "Created";
    case 204:
        return "No Content";
    case 400:
        return "Bad Request";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 500:
        return "Internal Server Error";
    default:
        return "OK";
    }
}

void http_write_response(int fd, const struct http_response *res)
{
    char header[256];
    int n = snprintf(header, sizeof header,
                     "HTTP/1.1 %d %s\r\n"
                     "Content-Type: %s\r\n"
                     "Content-Length: %zu\r\n"
                     "Connection: close\r\n"
                     "\r\n",
                     res->status, status_text(res->status), res->content_type,
                     res->body_len);

    if (n > 0) {
        (void)write(fd, header, (size_t)n);
    }
    if (res->body_len > 0) {
        (void)write(fd, res->body, res->body_len);
    }
}

int http_listen_tcp(int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    int yes = 1;

    if (fd < 0) {
        return -1;
    }

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons((uint16_t)port);

    if (bind(fd, (struct sockaddr *)&addr, sizeof addr) != 0 ||
        listen(fd, 16) != 0) {
        close(fd);
        return -1;
    }

    return fd;
}

int http_local_port(int listener)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof addr;

    if (getsockname(listener, (struct sockaddr *)&addr, &len) != 0) {
        return -1;
    }

    return ntohs(addr.sin_port);
}

void http_serve_connection(int conn, http_handler handler, void *ctx)
{
    char raw[8192];
    struct http_request request;
    struct http_response response;
    ssize_t n = read(conn, raw, sizeof raw - 1);

    if (n <= 0) {
        return;
    }
    raw[n] = '\0';

    if (!http_parse_request(raw, (size_t)n, &request)) {
        http_response_text(&response, 400, "bad request");
    } else {
        handler(&request, &response, ctx);
    }

    http_write_response(conn, &response);
}
