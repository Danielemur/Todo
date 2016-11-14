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

static void *add_element(void *base, size_t *nmemb, size_t size, unsigned i, void **new_elem)
{
    base = realloc(base, ++(*nmemb) * size);
    memmove((char *)base + (i + 1) * size,
            (char *)base + i * size,
            (*nmemb - (i + 1)) * size);
    *new_elem = ((char *)base + i * size);
    return base;
}

static void remove_element(void *base, size_t *nmemb, size_t size, unsigned i)
{
    (*nmemb)--;
    memmove((char *)base + i * size,
            (char *)base + (i + 1) * size,
            (*nmemb - i) * size);
}
