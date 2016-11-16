#pragma once

#include <stdlib.h>
#include <string.h>

#define COUNTOF(x) (sizeof(x) / sizeof(0[x]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define FATAL(args...)                          \
    do {                                        \
        fprintf(stderr, args);                \
        exit(EXIT_FAILURE);                     \
    } while(0)

static char *str_dup(const char *s)
{
    char *ret_str = malloc(strlen(s) + 1);
    if (ret_str == NULL)
        return ret_str;
    return strcpy(ret_str, s);
}

static unsigned strsubct(const char *str, const char *sub)
{
    unsigned count = 0;
    if (str && *str && sub && *sub) {
        while (str = strstr(str, sub)) {
            count++;
            str += strlen(sub);
        }
    }
    return count;
}

static char *strrepl(const char *str, const char *find, const char *repl)
{
    if (str && *str && find && *find && repl) {
        int flen = strlen(find);
        int rlen = strlen(repl);
        int diff = rlen - flen;
        char *retstr = malloc(strlen(str) + MAX(0, diff * strsubct(str, find)));
        str = strcpy(retstr, str);
        while (str = strstr(str, find)) {
            char *start = (char *)str + flen;
            memmove(start + diff, start, strlen(start));
            strncpy((char *)str, repl, rlen);
            str += rlen;
        }
        return retstr;
    }
    return NULL;
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
