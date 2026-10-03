#include "options.h"

#include <getopt.h>
#include <stdlib.h>
#include <string.h>

static void copy_str(char *dst, size_t cap, const char *src)
{
    size_t i = 0;

    for (; src[i] != '\0' && i + 1 < cap; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

bool parse_options(int argc, char **argv, struct options *out)
{
    int opt;

    memset(out, 0, sizeof *out);
    out->port = 0;

    opterr = 0;
    optind = 1;

    while ((opt = getopt(argc, argv, "p:s:")) != -1) {
        switch (opt) {
        case 'p':
            out->port = atoi(optarg);
            break;
        case 's':
            copy_str(out->store_path, sizeof out->store_path, optarg);
            out->persist = true;
            break;
        default:
            return false;
        }
    }

    return true;
}
