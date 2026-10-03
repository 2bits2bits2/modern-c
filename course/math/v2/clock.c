#include <stdio.h>

#include "hand.h"

#define CENTER 200.0
#define RADIUS 180.0

static void hand(FILE *out, double angle, double length, const char *colour)
{
    double x = 0;
    double y = 0;
    hand_point(angle, length, &x, &y);

    fprintf(out,
            "<line x1=\"%.0f\" y1=\"%.0f\" x2=\"%.0f\" y2=\"%.0f\" "
            "stroke=\"%s\" stroke-width=\"4\"/>\n",
            CENTER, CENTER, CENTER + x, CENTER + y, colour);
}

int main(void)
{
    double h = 0;
    double m = 0;
    double s = 0;

    clock_hand_angles(10, 10, 30, &h, &m, &s);

    printf("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"400\" "
           "height=\"400\">\n");
    printf("<circle cx=\"%.0f\" cy=\"%.0f\" r=\"%.0f\" fill=\"white\" "
           "stroke=\"black\"/>\n",
           CENTER, CENTER, RADIUS);
    hand(stdout, h, RADIUS * 0.5, "black");
    hand(stdout, m, RADIUS * 0.75, "black");
    hand(stdout, s, RADIUS * 0.9, "red");
    printf("</svg>\n");

    return 0;
}
