#ifndef VALUE_H
#define VALUE_H

enum value_kind {
    VALUE_INT,
    VALUE_DOUBLE,
    VALUE_STR,
};

/*
 * When you need a value that can be more than one type, you carry a tag and a
 * union. This is the C version of a sum type, and it is how many real systems
 * model dynamic data.
 */
struct value {
    enum value_kind kind;
    union {
        long i;
        double d;
        const char *s;
    };
};

const char *value_kind_name(const struct value *v);
double value_as_double(const struct value *v);

#endif /* VALUE_H */
