#ifndef READ_FILE_H
#define READ_FILE_H

#include <stddef.h>

/*
 * Read the whole of path into a freshly allocated, NUL-terminated buffer.
 * Returns NULL on error. On success the caller owns the buffer and must free
 * it. If out_len is not NULL it receives the number of bytes read.
 */
char *read_file(const char *path, size_t *out_len);

#endif /* READ_FILE_H */
