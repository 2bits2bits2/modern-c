#ifndef GENERIC_H
#define GENERIC_H

static inline int max_int(int a, int b)
{
    return a > b ? a : b;
}

static inline double max_double(double a, double b)
{
    return a > b ? a : b;
}

/* Pick the right implementation based on the type of a. */
#define max(a, b)                                                              \
    _Generic((a), int: max_int, double: max_double, default: max_int)((a), (b))

/* typeof is C23. It lets a macro declare a temporary of the same type. */
#define swap(a, b)                                                             \
    do {                                                                       \
        typeof(a) mctest_tmp = (a);                                            \
        (a) = (b);                                                             \
        (b) = mctest_tmp;                                                      \
    } while (0)

#endif /* GENERIC_H */
