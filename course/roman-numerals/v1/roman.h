#ifndef ROMAN_H
#define ROMAN_H

#include <stddef.h>

/* Writes the Roman numeral for n (1..3999) into buf. Returns the number of
 * characters written, or -1 if n is out of range or buf is too small. */
int to_roman(int n, char *buf, size_t cap);

/* Parses a Roman numeral. Returns the value, or -1 if it is not valid. */
int from_roman(const char *s);

#endif /* ROMAN_H */
