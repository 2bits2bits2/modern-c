#include <string.h>
#include <unistd.h>

#include "mctest.h"
#include "websocket.h"

static void test_accept_key_matches_rfc_example(void)
{
    char accept[64];

    websocket_accept_key("dGhlIHNhbXBsZSBub25jZQ==", accept, sizeof accept);
    CHECK_STR(accept, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=");
}

static void test_encode_text_frame(void)
{
    unsigned char frame[16];

    size_t n = websocket_encode_text("Hi", 2, frame, sizeof frame);

    CHECK_INT(n, 4);
    CHECK_INT(frame[0], 0x81); /* FIN + text */
    CHECK_INT(frame[1], 0x02); /* unmasked, length 2 */
    CHECK_INT(frame[2], 'H');
    CHECK_INT(frame[3], 'i');
}

static void test_decode_masked_frame(void)
{
    unsigned char mask[4] = {0x37, 0xfa, 0x21, 0x3d};
    const char *message = "Hello";
    unsigned char frame[16];
    size_t n = 0;
    int pipefd[2];
    char payload[32];

    CHECK_INT(pipe(pipefd), 0);

    frame[n++] = 0x81;
    frame[n++] = 0x80 | (unsigned char)strlen(message); /* masked */
    memcpy(frame + n, mask, 4);
    n += 4;
    for (size_t i = 0; message[i] != '\0'; i++) {
        frame[n++] = (unsigned char)message[i] ^ mask[i % 4];
    }

    CHECK_INT(write(pipefd[1], frame, n), (int)n);
    CHECK_INT(websocket_read_frame(pipefd[0], payload, sizeof payload), 0x1);
    CHECK_STR(payload, "Hello");

    close(pipefd[0]);
    close(pipefd[1]);
}

int main(void)
{
    RUN_TEST(test_accept_key_matches_rfc_example);
    RUN_TEST(test_encode_text_frame);
    RUN_TEST(test_decode_masked_frame);
    return test_summary();
}
