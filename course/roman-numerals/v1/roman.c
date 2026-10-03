#include "roman.h"

#include <string.h>

static const struct {
    int value;
    const char *symbol;
} symbols[] = {
    {1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"}, {100, "C"},
    {90, "XC"},  {50, "L"},   {40, "XL"}, {10, "X"},   {9, "IX"},
    {5, "V"},    {4, "IV"},   {1, "I"},
};

int to_roman(int n, char *buf, size_t cap)
{
    if (n < 1 || n > 3999) {
        return -1;
    }

    size_t pos = 0;
    for (size_t i = 0; i < sizeof symbols / sizeof symbols[0]; i++) {
        while (n >= symbols[i].value) {
            size_t len = strlen(symbols[i].symbol);
            if (pos + len + 1 > cap) {
                return -1;
            }
            memcpy(buf + pos, symbols[i].symbol, len);
            pos += len;
            n -= symbols[i].value;
        }
    }

    buf[pos] = '\0';
    return (int)pos;
}

int from_roman(const char *s)
{
    int total = 0;
    int prev = 0;
    size_t len = strlen(s);

    if (len == 0) {
        return -1;
    }

    for (size_t i = len; i-- > 0;) {
        int value;
        switch (s[i]) {
        case 'I':
            value = 1;
            break;
        case 'V':
            value = 5;
            break;
        case 'X':
            value = 10;
            break;
        case 'L':
            value = 50;
            break;
        case 'C':
            value = 100;
            break;
        case 'D':
            value = 500;
            break;
        case 'M':
            value = 1000;
            break;
        default:
            return -1;
        }

        if (value < prev) {
            total -= value;
        } else {
            total += value;
            prev = value;
        }
    }

    return total;
}
