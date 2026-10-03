#include "template.h"

#include <string.h>

static const char *lookup(const struct binding *bindings, size_t n,
                          const char *key, size_t key_len)
{
    for (size_t i = 0; i < n; i++) {
        if (strlen(bindings[i].key) == key_len &&
            strncmp(bindings[i].key, key, key_len) == 0) {
            return bindings[i].value;
        }
    }
    return NULL;
}

void render_all(const char *tmpl, const struct binding *bindings, size_t n,
                char *buf, size_t cap)
{
    size_t pos = 0;
    size_t i = 0;

    while (tmpl[i] != '\0' && pos + 1 < cap) {
        if (tmpl[i] == '{') {
            const char *close = strchr(tmpl + i + 1, '}');
            const char *value = close != NULL
                                    ? lookup(bindings, n, tmpl + i + 1,
                                             (size_t)(close - (tmpl + i + 1)))
                                    : NULL;

            if (value != NULL) {
                for (size_t j = 0; value[j] != '\0' && pos + 1 < cap; j++) {
                    buf[pos] = value[j];
                    pos++;
                }
                i = (size_t)(close - tmpl) + 1;
                continue;
            }
        }

        buf[pos] = tmpl[i];
        pos++;
        i++;
    }

    if (cap > 0) {
        buf[pos] = '\0';
    }
}
