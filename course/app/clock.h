#ifndef CLOCK_H
#define CLOCK_H

/*
 * A clock is a function pointer plus a context, exactly like the test doubles
 * from the mocking chapter. Production code uses clock_system(); tests use a
 * fake they control.
 */
struct clock {
    long (*now_ms)(void *ctx);
    void *ctx;
};

long clock_now_ms(const struct clock *clock);

/* The real, monotonic system clock. */
struct clock clock_system(void);

/* A clock whose value only changes when a test says so. */
struct fake_clock {
    long now_ms;
};

long fake_clock_now(void *ctx);
struct clock fake_clock_as_clock(struct fake_clock *fake);
void fake_clock_advance(struct fake_clock *fake, long ms);

#endif /* CLOCK_H */
