#ifndef RENDER_H
#define RENDER_H

#include <stddef.h>

/*
 * Replace every occurrence of {key} in tmpl with value, writing the result to
 * buf (NUL-terminated, never more than cap bytes including the terminator).
 */
void render(const char *tmpl, const char *key, const char *value, char *buf,
            size_t cap);

#endif /* RENDER_H */
