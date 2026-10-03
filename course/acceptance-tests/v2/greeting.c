#include "greeting.h"

void greeting(const char *name, char *buf, size_t n)
{
    const char *parts[3] = {"Hello, ", name, "!"};
    size_t pos = 0;

    if (n == 0) {
        return;
    }

    for (int p = 0; p < 3; p++) {
        for (const char *s = parts[p]; *s != '\0' && pos + 1 < n; s++) {
            buf[pos] = *s;
            pos++;
        }
    }

    buf[pos] = '\0';
}
