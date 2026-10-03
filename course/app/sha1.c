#include "sha1.h"

#include <stdint.h>
#include <string.h>

#define ROL32(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

static void process_block(uint32_t h[5], const unsigned char block[64])
{
    uint32_t w[80];
    uint32_t a, b, c, d, e;

    for (int i = 0; i < 16; i++) {
        w[i] = ((uint32_t)block[i * 4] << 24) |
               ((uint32_t)block[i * 4 + 1] << 16) |
               ((uint32_t)block[i * 4 + 2] << 8) | (uint32_t)block[i * 4 + 3];
    }
    for (int i = 16; i < 80; i++) {
        w[i] = ROL32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    a = h[0];
    b = h[1];
    c = h[2];
    d = h[3];
    e = h[4];

    for (int i = 0; i < 80; i++) {
        uint32_t f, k, tmp;

        if (i < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }

        tmp = ROL32(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = ROL32(b, 30);
        b = a;
        a = tmp;
    }

    h[0] += a;
    h[1] += b;
    h[2] += c;
    h[3] += d;
    h[4] += e;
}

void sha1(const unsigned char *data, size_t len, unsigned char out[20])
{
    uint32_t h[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476,
                     0xC3D2E1F0};
    unsigned char tail[128];
    size_t i = 0;
    size_t remaining;
    size_t total;
    size_t pad;
    uint64_t bits;

    for (; i + 64 <= len; i += 64) {
        process_block(h, data + i);
    }

    remaining = len - i;
    memcpy(tail, data + i, remaining);
    tail[remaining] = 0x80;
    total = remaining + 1;
    pad = (total % 64 <= 56) ? (56 - total % 64) : (120 - total % 64);
    memset(tail + total, 0, pad);
    total += pad;

    bits = (uint64_t)len * 8;
    for (int j = 0; j < 8; j++) {
        tail[total + j] = (unsigned char)(bits >> (56 - 8 * j));
    }
    total += 8;

    for (size_t k = 0; k < total; k += 64) {
        process_block(h, tail + k);
    }

    for (int j = 0; j < 5; j++) {
        out[j * 4] = (unsigned char)((h[j] >> 24) & 0xff);
        out[j * 4 + 1] = (unsigned char)((h[j] >> 16) & 0xff);
        out[j * 4 + 2] = (unsigned char)((h[j] >> 8) & 0xff);
        out[j * 4 + 3] = (unsigned char)(h[j] & 0xff);
    }
}
