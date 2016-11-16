#pragma once

#include <stdio.h>

char *csv_next_tok(char **line);
long  csv_get_line(char **lineptr, size_t *n, FILE *stream);
