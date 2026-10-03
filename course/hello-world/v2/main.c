#include <stdio.h>

#include "hello.h"

int main(void)
{
    char buf[64];

    hello("world", buf, sizeof buf);
    puts(buf);
    return 0;
}
