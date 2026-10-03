#include "greet.h"

void greet(FILE *out, const char *name)
{
    fprintf(out, "Hello, %s!", name);
}
