#pragma once

#include <stdio.h>

long  csv_cat_tok(char **line, size_t *size, const char *tok);
char *csv_next_tok(char **line);
long  csv_get_row(char **line, size_t *n, FILE *stream);
