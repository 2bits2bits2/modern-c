#ifndef RECTANGLE_H
#define RECTANGLE_H

struct rectangle {
    double width;
    double height;
};

double rectangle_area(struct rectangle r);
double rectangle_perimeter(struct rectangle r);

#endif /* RECTANGLE_H */
