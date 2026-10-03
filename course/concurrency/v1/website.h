#ifndef WEBSITE_H
#define WEBSITE_H

#include <stdbool.h>
#include <stddef.h>

struct website {
    const char *url;
    bool ok;
};

typedef bool (*check_fn)(const char *url);

void check_all_sequential(struct website *sites, size_t n, check_fn check);
void check_all_concurrent(struct website *sites, size_t n, check_fn check);

#endif /* WEBSITE_H */
