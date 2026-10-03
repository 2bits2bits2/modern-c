#include "mctest.h"
#include "shapes.h"

#define PI 3.14159265358979323846

struct shape_case {
    struct shape shape;
    double want_area;
    double want_perimeter;
};

static void test_shapes(void)
{
    struct rectangle r = {.width = 3, .height = 4};
    struct circle c = {.radius = 2};

    struct shape_case cases[] = {
        {.shape = rectangle_as_shape(&r),
         .want_area = 12,
         .want_perimeter = 14},
        {.shape = circle_as_shape(&c),
         .want_area = PI * 4,
         .want_perimeter = 4 * PI},
    };

    size_t n = sizeof cases / sizeof cases[0];
    for (size_t i = 0; i < n; i++) {
        CHECK_DOUBLE(shape_area(cases[i].shape), cases[i].want_area, 1e-9);
        CHECK_DOUBLE(shape_perimeter(cases[i].shape), cases[i].want_perimeter,
                     1e-9);
    }
}

int main(void)
{
    RUN_TEST(test_shapes);
    return test_summary();
}
