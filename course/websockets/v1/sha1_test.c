#include <string.h>

#include "mctest.h"
#include "sha1.h"

static void to_hex(const unsigned char *data, size_t len, char *out)
{
    static const char *digits = "0123456789abcdef";

    for (size_t i = 0; i < len; i++) {
        out[i * 2] = digits[data[i] >> 4];
        out[i * 2 + 1] = digits[data[i] & 0x0f];
    }
    out[len * 2] = '\0';
}

static void test_sha1_vectors(void)
{
    unsigned char digest[20];
    char hex[41];

    /* FIPS 180-1 test vectors. */
    sha1((const unsigned char *)"abc", 3, digest);
    to_hex(digest, 20, hex);
    CHECK_STR(hex, "a9993e364706816aba3e25717850c26c9cd0d89d");

    sha1((const unsigned char *)"", 0, digest);
    to_hex(digest, 20, hex);
    CHECK_STR(hex, "da39a3ee5e6b4b0d3255bfef95601890afd80709");

    sha1((const unsigned char
              *)"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
         56, digest);
    to_hex(digest, 20, hex);
    CHECK_STR(hex, "84983e441c3bd26ebaae4aa1f95129e5e54670f1");
}

int main(void)
{
    RUN_TEST(test_sha1_vectors);
    return test_summary();
}
