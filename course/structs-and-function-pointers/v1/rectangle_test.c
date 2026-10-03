#include "mctest.h"
#include "rectangle.h"

static void test_area(void)
{
    struct rectangle r = {.width = 3, .height = 4};

    CHECK_DOUBLE(rectangle_area(r), 12, 1e-9);
}

static void test_perimeter(void)
{
    struct rectangle r = {.width = 3, .height = 4};

    CHECK_DOUBLE(rectangle_perimeter(r), 14, 1e-9);
}

int main(void)
{
    RUN_TEST(test_area);
    RUN_TEST(test_perimeter);
    return test_summary();
}
