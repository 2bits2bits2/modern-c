#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "website.h"

#define NUM_SITES 8

static bool slow_check(const char *url)
{
    struct timespec ts = {.tv_sec = 0, .tv_nsec = 50 * 1000 * 1000};
    (void)url;
    nanosleep(&ts, NULL);
    return true;
}

static double seconds_between(struct timespec start, struct timespec end)
{
    return (double)(end.tv_sec - start.tv_sec) +
           (double)(end.tv_nsec - start.tv_nsec) / 1e9;
}

static double time_sequential(struct website *sites, size_t n)
{
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    check_all_sequential(sites, n, slow_check);
    clock_gettime(CLOCK_MONOTONIC, &end);

    return seconds_between(start, end);
}

static double time_concurrent(struct website *sites, size_t n)
{
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    check_all_concurrent(sites, n, slow_check);
    clock_gettime(CLOCK_MONOTONIC, &end);

    return seconds_between(start, end);
}

int main(void)
{
    struct website sites[NUM_SITES];

    for (int i = 0; i < NUM_SITES; i++) {
        sites[i] = (struct website){.url = "https://example.com", .ok = false};
    }

    printf("sequential: %.3f s\n", time_sequential(sites, NUM_SITES));
    printf("concurrent: %.3f s\n", time_concurrent(sites, NUM_SITES));
    return 0;
}
