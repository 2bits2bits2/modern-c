#ifndef HTTP_H
#define HTTP_H

#include <stdbool.h>
#include <stddef.h>

#define HTTP_PATH_MAX 256
#define HTTP_BODY_MAX 4096

struct http_request {
    char method[8];
    char path[HTTP_PATH_MAX];
    char body[HTTP_BODY_MAX];
    size_t body_len;
};

struct http_response {
    int status;
    char content_type[64];
    char body[HTTP_BODY_MAX];
    size_t body_len;
};

/* A handler receives a parsed request and fills in a response. */
typedef void (*http_handler)(const struct http_request *request,
                             struct http_response *response, void *ctx);

void http_response_init(struct http_response *res, int status,
                        const char *content_type);
void http_response_text(struct http_response *res, int status,
                        const char *text);
void http_response_json(struct http_response *res, int status,
                        const char *json);

/* Parse the request line and body from raw bytes. Returns false if the
 * request line is malformed. */
bool http_parse_request(const char *raw, size_t len, struct http_request *out);

/* Serialise a response and write it to fd. */
void http_write_response(int fd, const struct http_response *res);

/* Create a listening TCP socket bound to 127.0.0.1:port. Port 0 asks the OS
 * to choose. Returns the descriptor, or -1. */
int http_listen_tcp(int port);

/* The port a listening socket is bound to. Returns -1 on error. */
int http_local_port(int listener);

/* Read one request from conn, run the handler, write the response. */
void http_serve_connection(int conn, http_handler handler, void *ctx);

#endif /* HTTP_H */
