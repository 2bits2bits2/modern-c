#ifndef DESCRIBE_H
#define DESCRIBE_H

/*
 * C23 has no runtime type information worth the name. What it does have is
 * _Generic: a compile-time switch on the type of an expression. That is as
 * close to "reflection" as we get, and it is surprisingly useful.
 */
#define describe(x)                                                            \
    _Generic((x),                                                              \
        int: "int",                                                            \
        long: "long",                                                          \
        double: "double",                                                      \
        float: "float",                                                        \
        char *: "string",                                                      \
        const char *: "string",                                                \
        default: "unknown")

#endif /* DESCRIBE_H */
