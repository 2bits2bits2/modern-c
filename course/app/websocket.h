#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <stddef.h>

/* Compute the Sec-WebSocket-Accept value for a client's key. */
void websocket_accept_key(const char *client_key, char *out, size_t cap);

/* Encode an unmasked text frame (server to client). Returns bytes written. */
size_t websocket_encode_text(const char *payload, size_t len,
                             unsigned char *out, size_t cap);

/* Read one frame from fd. Unmasks the payload and NUL-terminates it. Returns
 * the opcode (0x1 text, 0x8 close, ...) or -1 on error/EOF. */
int websocket_read_frame(int fd, char *payload, size_t cap);

int websocket_write_text(int fd, const char *payload, size_t len);
int websocket_write_close(int fd);

#endif /* WEBSOCKET_H */
