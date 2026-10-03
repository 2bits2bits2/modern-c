#include "repeat.h"

void repeat(const char *s, int times, char *buf, size_t n)
{
    size_t pos = 0;

    for (int i = 0; i < times; i++) {
        for (size_t j = 0; s[j] != '\0' && pos + 1 < n; j++) {
            buf[pos] = s[j];
            pos++;
        }
    }

    if (n > 0) {
        buf[pos] = '\0';
    }
}
