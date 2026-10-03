#ifndef WAITGROUP_H
#define WAITGROUP_H

#include <pthread.h>

struct waitgroup {
    pthread_mutex_t mu;
    pthread_cond_t cond;
    int count;
};

void wg_init(struct waitgroup *wg);
void wg_destroy(struct waitgroup *wg);
void wg_add(struct waitgroup *wg, int n);
void wg_done(struct waitgroup *wg);
void wg_wait(struct waitgroup *wg);

#endif /* WAITGROUP_H */
