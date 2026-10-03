#include "value.h"

const char *value_kind_name(const struct value *v)
{
    switch (v->kind) {
    case VALUE_INT:
        return "int";
    case VALUE_DOUBLE:
        return "double";
    case VALUE_STR:
        return "string";
    }
    return "unknown";
}

double value_as_double(const struct value *v)
{
    switch (v->kind) {
    case VALUE_INT:
        return (double)v->i;
    case VALUE_DOUBLE:
        return v->d;
    case VALUE_STR:
        return 0;
    }
    return 0;
}
