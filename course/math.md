# Maths

**[You can find all the code for this chapter here](math/)**

We are going to draw an analog clock as an SVG. To do that we need to place the
hands at an angle, which means trigonometry, which means `<math.h>` and a
short detour into how C handles floating point and angles.

## Just enough trig

A point at angle θ and distance `r` from the centre is:

```
x = r * sin(θ)
y = -r * cos(θ)
```

The minus on `y` is because screen coordinates grow *downwards*: 0° is at the
top of the clock, not the bottom. Angles in `<math.h>` are in **radians**, and
there are `2π` of them in a full turn. Since clock angles are more naturally
degrees, we convert: `radians = degrees * π / 180`.

We define our own `π` rather than relying on `M_PI`, which is a POSIX extension
rather than standard C, and so is not available in strict ISO mode.

## Write the test first

The cardinal angles are easy to reason about, so test those:

```c
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

    /* ... 180 and 270 ... */
}
```

`hand_point` writes through two out-parameters because C functions return one
value. That is exactly the `(char *buf, size_t n)` pattern from
[Hello, World](hello-world.md), applied to doubles.

We also need to work out each hand's angle:

```c
void clock_hand_angles(int hour, int minute, int second, double *hour_deg,
                       double *minute_deg, double *second_deg)
{
    *second_deg = second * 6.0;
    *minute_deg = minute * 6.0 + second * 0.1;
    *hour_deg = (hour % 12) * 30.0 + minute * 0.5;
}
```

The second hand moves 6° per second (360/60). The minute hand moves 6° per
minute *plus* a little for the seconds, so it is not stuck pointing exactly at
the minute. The hour hand moves 30° per hour *plus* half a degree per minute.
Those fractional terms are what make a clock look right rather than tick like a
cheap digital watch.

## Write enough code to make it pass

```c
void hand_point(double angle_deg, double length, double *x, double *y)
{
    double radians = angle_deg * PI / 180.0;
    *x = length * sin(radians);
    *y = -length * cos(radians);
}
```

Run the tests. Green, and the cardinal angles give exactly the numbers you would
sketch on paper.

## Rendering the clock

The drawing is just `printf`, because SVG is text. Here is the interesting part:

```c
static void hand(FILE *out, double angle, double length, const char *colour)
{
    double x = 0;
    double y = 0;
    hand_point(angle, length, &x, &y);

    fprintf(out, "<line x1=\"%.0f\" y1=\"%.0f\" x2=\"%.0f\" y2=\"%.0f\" "
                 "stroke=\"%s\" stroke-width=\"4\"/>\n",
            CENTER, CENTER, CENTER + x, CENTER + y, colour);
}
```

Build and run it:

```sh
cmake --build build --target math_v2_clock
./build/math/math_v2_clock > clock.svg
```

You now have a real, viewable clock face. Open it in a browser. Changing the
time in `clock.c` and re-running redraws the hands — because the geometry is
tested, you can play with the drawing without fear.

## Refactor: keep the maths and the rendering apart

Notice that all the logic worth testing — angles, points — lives in `hand.c`,
and the rendering lives in `clock.c`, which is a program, not a library. That
split is the whole trick. If we had inlined the trigonometry into `printf`
calls, testing it would have meant parsing strings.

This is the same separation the [dependency injection](dependency-injection.md)
chapter made: push the interesting values out to where a test can see them, and
keep the side effects at the edges.

## Wrapping up

What we have covered:

- Degrees to radians, and screen coordinates with `y` downwards
- Using `<math.h>` (`sin`, `cos`) and linking `-lm`
- Out-parameters for returning more than one value
- Fractional hand angles so the clock looks real
- Separating testable maths from side-effecting rendering

The Go version of this chapter calls it "the `math` package"; here `sin` and
`cos` have been in C since 1978. Some things do not need improving.

### Additional material

- [`<math.h>` reference](https://en.cppreference.com/w/c/numeric/math)
- [SVG line element](https://developer.mozilla.org/en-US/docs/Web/SVG/Element/line)
