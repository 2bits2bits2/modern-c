#include "waitgroup.h"

void wg_init(struct waitgroup *wg)
{
    pthread_mutex_init(&wg->mu, NULL);
    pthread_cond_init(&wg->cond, NULL);
    wg->count = 0;
}

void wg_destroy(struct waitgroup *wg)
{
    pthread_mutex_destroy(&wg->mu);
    pthread_cond_destroy(&wg->cond);
}

void wg_add(struct waitgroup *wg, int n)
{
    pthread_mutex_lock(&wg->mu);
    wg->count += n;
    pthread_mutex_unlock(&wg->mu);
}

void wg_done(struct waitgroup *wg)
{
    pthread_mutex_lock(&wg->mu);
    wg->count--;
    if (wg->count == 0) {
        pthread_cond_broadcast(&wg->cond);
    }
    pthread_mutex_unlock(&wg->mu);
}

void wg_wait(struct waitgroup *wg)
{
    pthread_mutex_lock(&wg->mu);
    while (wg->count > 0) {
        pthread_cond_wait(&wg->cond, &wg->mu);
    }
    pthread_mutex_unlock(&wg->mu);
}
