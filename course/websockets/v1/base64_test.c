#include <string.h>

#include "base64.h"
#include "mctest.h"

static void test_base64_vectors(void)
{
    static const struct {
        const char *input;
        const char *want;
    } cases[] = {
        {"", ""},
        {"f", "Zg=="},
        {"fo", "Zm8="},
        {"foo", "Zm9v"},
        {"foob", "Zm9vYg=="},
        {"fooba", "Zm9vYmE="},
        {"foobar", "Zm9vYmFy"},
    };

    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        char out[64];

        base64_encode((const unsigned char *)cases[i].input,
                      strlen(cases[i].input), out, sizeof out);
        CHECK_STR(out, cases[i].want);
    }
}

int main(void)
{
    RUN_TEST(test_base64_vectors);
    return test_summary();
}
