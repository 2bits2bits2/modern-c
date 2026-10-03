#ifndef HAND_H
#define HAND_H

/*
 * Angles are in degrees, measured clockwise from 12 o'clock, the way a clock
 * face is laid out. The point is computed in screen coordinates: x grows to
 * the right, y grows downwards.
 */
void hand_point(double angle_deg, double length, double *x, double *y);

void clock_hand_angles(int hour, int minute, int second, double *hour_deg,
                       double *minute_deg, double *second_deg);

#endif /* HAND_H */
