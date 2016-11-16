#include "csv.h"

#include <stdbool.h>
#include <stdlib.h>

static inline bool term_val(char c)
{
    return c == ',' || c == '\0' || c == '\n';
}

char *csv_next_tok(char **line)
{
    if (!line || !*line || !**line)
        return NULL;

    const bool dqted = **line == '"';
    *line += dqted;
    char *start = *line;

    for (; dqted || !term_val(**line); (*line)++) {
        if (dqted) {
            if (**line == '"') {
                if (*(*line + 1) == '"') {
                    (*line)++;
                    continue;
                } else if (term_val(*(*line + 1))) {
                    **line = '\0';
                    (*line)++;
                    break;
                } else
                    return NULL;
            }
        } else if (**line == '"') {
            return NULL;
        }
    }

    if (**line == ',' || **line == '\n') {
        **line = '\0';
        (*line)++;
    }

    return start;
}

long csv_get_line(char **lineptr, size_t *n, FILE *stream)
{
    if (*lineptr == NULL) {
        *n = 4;
        *lineptr = malloc(*n);
    }

    bool qtd = false;

    int k;
    size_t c = 0;
    for (k = fgetc(stream); k != EOF && (k != '\n' || qtd); c++, k = fgetc(stream)){
        if (k == '"')
            qtd = !qtd;

        if (c >= *n - 2) {
            *n *= 2;
            *lineptr = realloc(*lineptr, *n);
        }
        (*lineptr)[c] = k;
    }

    if (c == 0 && k == EOF) {
        return -1;
    } else if (k == EOF) {
        if (ferror(stream))
            return -1;
    } else if (k == '\n') {
        (*lineptr)[c] = '\n';
        c++;
    }

    (*lineptr)[c] = '\0';
    return c;
}
