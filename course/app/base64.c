#include "base64.h"

#include <stdint.h>

static const char *table =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char *data, size_t len, char *out, size_t cap)
{
    size_t in = 0;
    size_t o = 0;

    while (in + 3 <= len) {
        uint32_t n = ((uint32_t)data[in] << 16) |
                     ((uint32_t)data[in + 1] << 8) | (uint32_t)data[in + 2];

        if (o + 4 < cap) {
            out[o++] = table[(n >> 18) & 63];
            out[o++] = table[(n >> 12) & 63];
            out[o++] = table[(n >> 6) & 63];
            out[o++] = table[n & 63];
        }
        in += 3;
    }

    if (in < len && o + 4 < cap) {
        int remaining = (int)(len - in);
        uint32_t n = (uint32_t)data[in] << 16;

        if (remaining == 2) {
            n |= (uint32_t)data[in + 1] << 8;
        }

        out[o++] = table[(n >> 18) & 63];
        out[o++] = table[(n >> 12) & 63];
        out[o++] = (remaining == 2) ? table[(n >> 6) & 63] : '=';
        out[o++] = '=';
    }

    if (o < cap) {
        out[o] = '\0';
    }
}
