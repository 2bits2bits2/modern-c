#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <stddef.h>

struct binding {
    const char *key;
    const char *value;
};

/*
 * Render tmpl, replacing {key} with the matching binding's value. Unknown
 * placeholders are left untouched.
 */
void render_all(const char *tmpl, const struct binding *bindings, size_t n,
                char *buf, size_t cap);

#endif /* TEMPLATE_H */
