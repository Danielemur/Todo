#pragma once

#include <stdlib.h>
#include <string.h>

static char *str_dup(const char *s)
{
    char *ret_str = malloc(strlen(s) + 1);
    if (ret_str == NULL)
        return ret_str;
    return strcpy(ret_str, s);
}
