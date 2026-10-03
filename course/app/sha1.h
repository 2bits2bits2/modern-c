#ifndef SHA1_H
#define SHA1_H

#include <stddef.h>

/* Compute the SHA-1 digest of data into out[20]. */
void sha1(const unsigned char *data, size_t len, unsigned char out[20]);

#endif /* SHA1_H */
