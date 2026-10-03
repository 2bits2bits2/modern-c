#include "greet.h"

void greet(struct writer out, const char *name)
{
    writer_write(out, "Hello, ");
    writer_write(out, name);
    writer_write(out, "!");
}
