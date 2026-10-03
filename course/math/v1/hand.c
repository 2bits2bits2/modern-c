#include "hand.h"

#include <math.h>

#define PI 3.14159265358979323846

void hand_point(double angle_deg, double length, double *x, double *y)
{
    double radians = angle_deg * PI / 180.0;
    *x = length * sin(radians);
    *y = -length * cos(radians);
}

void clock_hand_angles(int hour, int minute, int second, double *hour_deg,
                       double *minute_deg, double *second_deg)
{
    *second_deg = second * 6.0;
    *minute_deg = minute * 6.0 + second * 0.1;
    *hour_deg = (hour % 12) * 30.0 + minute * 0.5;
}
