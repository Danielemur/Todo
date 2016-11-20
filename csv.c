#include "csv.h"

#include <stdbool.h>
#include <stdlib.h>

#include "common.h"

long csv_cat_tok(char **line, size_t *size, const char *tok)
{
    if (!line || !size)
        return -1;

    tok = strrepl(tok, "\"", "\"\"");
    char *tofree = (char *)tok;
    tok = addqt(tok);
    free(tofree);

    size_t new_size = *size + strlen(tok) + (!*size ? 1 : 2);
    *line = realloc(*line, new_size);

    if (!*size)
        (*line)[0] = '\0';
    else
        strcat(*line, ",");

    *size = new_size;
    strcat(*line, tok);
}

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
    char *end = *line;
    char *ret;

    for (; dqted || !term_val(**line); end = ++*line) {
        if (dqted) {
            if (**line == '"') {
                if (*(*line + 1) == '"') {
                    end = ++*line;
                    continue;
                } else if (term_val(*(*line + 1))) {
                    end = *line;
                    (*line)++;
                    break;
                } else
                    return NULL;
            } else if (!**line) {
                return NULL;
            }
        } else if (**line == '"') {
            return NULL;
        }
    }

    ret = malloc(1 + end - start);
    memcpy(ret, start, end - start);
    ret[end - start] = '\0';

    if (**line == ',' || **line == '\n')
        (*line)++;

    if (strsubct(ret, "\"\"") > 0) {
        char *tofree = ret;
        ret = strrepl(ret, "\"\"", "\"");
        free(tofree);
        return ret;
    }
    else {
        return ret;
    }
}

long csv_get_row(char **line, size_t *n, FILE *stream)
{
    if (!*line) {
        *n = 4;
        *line = malloc(*n);
    }

    bool qtd = false;

    int k;
    size_t c = 0;
    for (k = fgetc(stream); k != EOF && (k != '\n' || qtd); c++, k = fgetc(stream)){
        if (k == '"')
            qtd = !qtd;

        if (c >= *n - 2) {
            *n *= 2;
            *line = realloc(*line, *n);
        }
        (*line)[c] = k;
    }

    if (c == 0 && k == EOF) {
        return -1;
    } else if (k == EOF) {
        if (ferror(stream))
            return -1;
    } else if (k == '\n') {
        (*line)[c] = '\n';
        c++;
    }

    (*line)[c] = '\0';
    return c;
}
