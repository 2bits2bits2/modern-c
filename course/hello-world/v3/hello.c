#include "hello.h"

#include <stdio.h>

void hello(const char *name, char *buf, size_t n)
{
    if (name[0] == '\0') {
        name = "World";
    }

    snprintf(buf, n, "Hello, %s", name);
}
