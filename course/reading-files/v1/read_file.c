#include "read_file.h"

#include <stdio.h>
#include <stdlib.h>

char *read_file(const char *path, size_t *out_len)
{
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }

    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);

    char *buf = malloc((size_t)size + 1);
    if (buf == NULL) {
        fclose(f);
        return NULL;
    }

    size_t got = fread(buf, 1, (size_t)size, f);
    fclose(f);

    if (got != (size_t)size) {
        free(buf);
        return NULL;
    }

    buf[got] = '\0';
    if (out_len != NULL) {
        *out_len = got;
    }
    return buf;
}
