#ifndef BASE64_H
#define BASE64_H

#include <stddef.h>

/* Encode bytes as Base64 into out (NUL-terminated). Needs cap > encoded size.
 */
void base64_encode(const unsigned char *data, size_t len, char *out,
                   size_t cap);

#endif /* BASE64_H */
