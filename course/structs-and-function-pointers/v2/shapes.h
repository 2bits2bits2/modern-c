#ifndef SHAPES_H
#define SHAPES_H

struct rectangle {
    double width;
    double height;
};

struct circle {
    double radius;
};

/*
 * A "shape" is an opaque pointer to some concrete value together with a
 * vtable. This is how C does interfaces: a struct of function pointers.
 */
struct shape_vtable {
    double (*area)(const void *self);
    double (*perimeter)(const void *self);
};

struct shape {
    const struct shape_vtable *vtable;
    const void *self;
};

double shape_area(struct shape s);
double shape_perimeter(struct shape s);

struct shape rectangle_as_shape(const struct rectangle *r);
struct shape circle_as_shape(const struct circle *c);

#endif /* SHAPES_H */
