#include "writer.h"

#include <string.h>

int writer_write(struct writer w, const char *s)
{
    return w.write(w.ctx, s, strlen(s));
}
