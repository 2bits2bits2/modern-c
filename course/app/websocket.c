#include "websocket.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "base64.h"
#include "sha1.h"

#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

void websocket_accept_key(const char *client_key, char *out, size_t cap)
{
    char combined[128];
    unsigned char digest[20];

    snprintf(combined, sizeof combined, "%s%s", client_key, WS_GUID);
    sha1((const unsigned char *)combined, strlen(combined), digest);
    base64_encode(digest, sizeof digest, out, cap);
}

size_t websocket_encode_text(const char *payload, size_t len,
                             unsigned char *out, size_t cap)
{
    size_t n = 0;

    if (cap < len + 2) {
        return 0;
    }

    out[n++] = 0x81; /* FIN + text opcode */

    if (len < 126) {
        out[n++] = (unsigned char)len;
    } else if (len < 65536) {
        out[n++] = 126;
        out[n++] = (unsigned char)((len >> 8) & 0xff);
        out[n++] = (unsigned char)(len & 0xff);
    } else {
        return 0;
    }

    memcpy(out + n, payload, len);
    return n + len;
}

static int read_exact(int fd, unsigned char *buf, size_t n)
{
    size_t got = 0;

    while (got < n) {
        ssize_t m = read(fd, buf + got, n - got);

        if (m <= 0) {
            return -1;
        }
        got += (size_t)m;
    }
    return 0;
}

static int write_exact(int fd, const unsigned char *buf, size_t n)
{
    size_t sent = 0;

    while (sent < n) {
        ssize_t m = write(fd, buf + sent, n - sent);

        if (m <= 0) {
            return -1;
        }
        sent += (size_t)m;
    }
    return 0;
}

int websocket_read_frame(int fd, char *payload, size_t cap)
{
    unsigned char header[2];
    unsigned char mask[4] = {0};
    unsigned char extra[8];
    uint64_t len;
    int opcode;
    int masked;

    if (read_exact(fd, header, 2) != 0) {
        return -1;
    }

    opcode = header[0] & 0x0f;
    masked = (header[1] & 0x80) != 0;
    len = header[1] & 0x7f;

    if (len == 126) {
        if (read_exact(fd, extra, 2) != 0) {
            return -1;
        }
        len = ((uint64_t)extra[0] << 8) | extra[1];
    } else if (len == 127) {
        if (read_exact(fd, extra, 8) != 0) {
            return -1;
        }
        len = 0;
        for (int i = 0; i < 8; i++) {
            len = (len << 8) | extra[i];
        }
    }

    if (masked && read_exact(fd, mask, 4) != 0) {
        return -1;
    }

    if (cap == 0) {
        return -1;
    }
    if (len > cap - 1) {
        len = cap - 1; /* truncate rather than overflow */
    }

    if (len > 0 && read_exact(fd, (unsigned char *)payload, (size_t)len) != 0) {
        return -1;
    }

    if (masked) {
        for (uint64_t i = 0; i < len; i++) {
            payload[i] ^= (char)mask[i % 4];
        }
    }
    payload[len] = '\0';
    return opcode;
}

int websocket_write_text(int fd, const char *payload, size_t len)
{
    unsigned char frame[1400];

    size_t n = websocket_encode_text(payload, len, frame, sizeof frame);
    if (n == 0) {
        return -1;
    }
    return write_exact(fd, frame, n);
}

int websocket_write_close(int fd)
{
    unsigned char frame[2] = {0x88, 0x00};

    return write_exact(fd, frame, sizeof frame);
}
