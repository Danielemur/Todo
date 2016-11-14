#pragma once

#include <stdlib.h>
#include <string.h>

#define COUNTOF(x) (sizeof(x) / sizeof(0[x]))

#define FATAL(x)                                \
    do {                                        \
        fprintf(stderror, "%s", (x));           \
        exit(EXIT_FAILURE);                     \
    } while(0)

static char *str_dup(const char *s)
{
    char *ret_str = malloc(strlen(s) + 1);
    if (ret_str == NULL)
        return ret_str;
    return strcpy(ret_str, s);
}
