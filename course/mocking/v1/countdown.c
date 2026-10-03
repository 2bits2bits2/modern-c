#include "countdown.h"

#include <stdio.h>

void countdown(struct writer out, struct sleeper s)
{
    for (int i = 3; i > 0; i--) {
        char line[4];
        snprintf(line, sizeof line, "%d\n", i);
        writer_write(out, line);
        s.sleep(s.ctx, 1);
    }

    writer_write(out, "Go!");
    s.sleep(s.ctx, 1);
}
