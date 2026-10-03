#include "shapes.h"

#define PI 3.14159265358979323846

static double rectangle_area(const void *self)
{
    const struct rectangle *r = self;
    return r->width * r->height;
}

static double rectangle_perimeter(const void *self)
{
    const struct rectangle *r = self;
    return 2 * (r->width + r->height);
}

static const struct shape_vtable rectangle_vtable = {
    .area = rectangle_area,
    .perimeter = rectangle_perimeter,
};

static double circle_area(const void *self)
{
    const struct circle *c = self;
    return PI * c->radius * c->radius;
}

static double circle_perimeter(const void *self)
{
    const struct circle *c = self;
    return 2 * PI * c->radius;
}

static const struct shape_vtable circle_vtable = {
    .area = circle_area,
    .perimeter = circle_perimeter,
};

double shape_area(struct shape s)
{
    return s.vtable->area(s.self);
}

double shape_perimeter(struct shape s)
{
    return s.vtable->perimeter(s.self);
}

struct shape rectangle_as_shape(const struct rectangle *r)
{
    return (struct shape){.vtable = &rectangle_vtable, .self = r};
}

struct shape circle_as_shape(const struct circle *c)
{
    return (struct shape){.vtable = &circle_vtable, .self = c};
}
