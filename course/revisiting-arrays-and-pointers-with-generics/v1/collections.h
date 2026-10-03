#ifndef COLLECTIONS_H
#define COLLECTIONS_H

#include "generic_array.h"

DEFINE_ARRAY(int_array, int)

int reduce_sum(const struct int_array *a);
struct int_array map_double_all(const struct int_array *a);
struct int_array filter_even(const struct int_array *a);

#endif /* COLLECTIONS_H */
