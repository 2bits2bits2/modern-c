#include "render.h"

#include <string.h>

void render(const char *tmpl, const char *key, const char *value, char *buf,
            size_t cap)
{
    size_t pos = 0;
    size_t i = 0;
    size_t key_len = strlen(key);

    while (tmpl[i] != '\0' && pos + 1 < cap) {
        if (tmpl[i] == '{' && strncmp(tmpl + i + 1, key, key_len) == 0 &&
            tmpl[i + 1 + key_len] == '}') {
            for (size_t j = 0; value[j] != '\0' && pos + 1 < cap; j++) {
                buf[pos] = value[j];
                pos++;
            }
            i += key_len + 2;
        } else {
            buf[pos] = tmpl[i];
            pos++;
            i++;
        }
    }

    if (cap > 0) {
        buf[pos] = '\0';
    }
}
