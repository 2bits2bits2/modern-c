#include "website.h"

#include <pthread.h>
#include <stdlib.h>

struct job {
    struct website *site;
    check_fn check;
};

static void *check_one(void *arg)
{
    struct job *j = arg;
    j->site->ok = j->check(j->site->url);
    return NULL;
}

void check_all_sequential(struct website *sites, size_t n, check_fn check)
{
    for (size_t i = 0; i < n; i++) {
        sites[i].ok = check(sites[i].url);
    }
}

void check_all_concurrent(struct website *sites, size_t n, check_fn check)
{
    pthread_t *threads = malloc(n * sizeof *threads);
    struct job *jobs = malloc(n * sizeof *jobs);

    if (threads == NULL || jobs == NULL) {
        free(threads);
        free(jobs);
        check_all_sequential(sites, n, check);
        return;
    }

    for (size_t i = 0; i < n; i++) {
        jobs[i] = (struct job){.site = &sites[i], .check = check};
        pthread_create(&threads[i], NULL, check_one, &jobs[i]);
    }

    for (size_t i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(jobs);
}
