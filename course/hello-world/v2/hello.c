#include "hello.h"

#include <stdio.h>

void hello(const char *name, char *buf, size_t n)
{
    snprintf(buf, n, "Hello, %s", name);
}
