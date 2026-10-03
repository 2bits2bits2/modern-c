#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

struct options {
    int port;
    bool persist;
    char store_path[256];
};

/*
 * Parse -p <port> and -s <store-path>. Remaining non-option arguments are
 * seed specifications. Returns false on a malformed option.
 */
bool parse_options(int argc, char **argv, struct options *out);

#endif /* OPTIONS_H */
