#include "mctest.h"
#include "value.h"

static void test_kind_name(void)
{
    const struct value i = {.kind = VALUE_INT, .i = 1};
    const struct value d = {.kind = VALUE_DOUBLE, .d = 1.5};
    const struct value s = {.kind = VALUE_STR, .s = "x"};

    CHECK_STR(value_kind_name(&i), "int");
    CHECK_STR(value_kind_name(&d), "double");
    CHECK_STR(value_kind_name(&s), "string");
}

static void test_as_double(void)
{
    const struct value i = {.kind = VALUE_INT, .i = 42};
    const struct value d = {.kind = VALUE_DOUBLE, .d = 1.5};

    CHECK_DOUBLE(value_as_double(&i), 42, 1e-9);
    CHECK_DOUBLE(value_as_double(&d), 1.5, 1e-9);
}

int main(void)
{
    RUN_TEST(test_kind_name);
    RUN_TEST(test_as_double);
    return test_summary();
}
