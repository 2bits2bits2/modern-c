#include <stdio.h>
#include <time.h>

#include "repeat.h"

static double seconds_between(struct timespec start, struct timespec end)
{
    return (double)(end.tv_sec - start.tv_sec) +
           (double)(end.tv_nsec - start.tv_nsec) / 1e9;
}

int main(void)
{
    char buf[1024];
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 1000000; i++) {
        repeat("ab", 10, buf, sizeof buf);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("repeat x 1,000,000 took %.3f ms\n",
           seconds_between(start, end) * 1000.0);
    return 0;
}
