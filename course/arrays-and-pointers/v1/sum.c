#include "sum.h"

long sum(const int *values, size_t n)
{
    long total = 0;

    for (size_t i = 0; i < n; i++) {
        total += values[i];
    }

    return total;
}
