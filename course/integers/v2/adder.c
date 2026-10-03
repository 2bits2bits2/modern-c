#include "adder.h"

int add(int a, int b)
{
    return a + b;
}

bool add_checked(int a, int b, int *result)
{
    return !__builtin_add_overflow(a, b, result);
}
