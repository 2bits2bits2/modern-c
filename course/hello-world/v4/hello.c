#include "hello.h"

#include <stdio.h>
#include <string.h>

static const char *greeting_prefix(const char *language)
{
    if (strcmp(language, "Spanish") == 0) {
        return "Hola, ";
    }
    if (strcmp(language, "French") == 0) {
        return "Bonjour, ";
    }
    return "Hello, ";
}

void hello(const char *name, const char *language, char *buf, size_t n)
{
    if (name[0] == '\0') {
        name = "World";
    }

    snprintf(buf, n, "%s%s", greeting_prefix(language), name);
}
