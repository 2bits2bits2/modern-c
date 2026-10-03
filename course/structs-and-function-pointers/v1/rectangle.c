#include "rectangle.h"

double rectangle_area(struct rectangle r)
{
    return r.width * r.height;
}

double rectangle_perimeter(struct rectangle r)
{
    return 2 * (r.width + r.height);
}
