#include "hand.h"
#include "mctest.h"

static void test_points_at_cardinal_angles(void)
{
    double x = 0;
    double y = 0;

    hand_point(0, 100, &x, &y);
    CHECK_DOUBLE(x, 0, 1e-9);
    CHECK_DOUBLE(y, -100, 1e-9);

    hand_point(90, 100, &x, &y);
    CHECK_DOUBLE(x, 100, 1e-9);
    CHECK_DOUBLE(y, 0, 1e-9);

    hand_point(180, 100, &x, &y);
    CHECK_DOUBLE(x, 0, 1e-9);
    CHECK_DOUBLE(y, 100, 1e-9);

    hand_point(270, 100, &x, &y);
    CHECK_DOUBLE(x, -100, 1e-9);
    CHECK_DOUBLE(y, 0, 1e-9);
}

static void test_hand_angles(void)
{
    double h = 0;
    double m = 0;
    double s = 0;

    clock_hand_angles(3, 0, 0, &h, &m, &s);
    CHECK_DOUBLE(h, 90, 1e-9);
    CHECK_DOUBLE(m, 0, 1e-9);
    CHECK_DOUBLE(s, 0, 1e-9);

    clock_hand_angles(12, 30, 0, &h, &m, &s);
    CHECK_DOUBLE(h, 15, 1e-9);
    CHECK_DOUBLE(m, 180, 1e-9);
}

int main(void)
{
    RUN_TEST(test_points_at_cardinal_angles);
    RUN_TEST(test_hand_angles);
    return test_summary();
}
