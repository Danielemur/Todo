#pragma once

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COUNTOF(x) (sizeof(x) / sizeof(0[x]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define FATAL(args...)                          \
    do {                                        \
        fprintf(stderr, args);                  \
        exit(EXIT_FAILURE);                     \
    } while(0)

#define RESET "\x1B[0m"
#define BOLD  "\x1B[1m"
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"

/* Boolean for colored output */
extern int TERM_COLOR;

#define PRTESC(args)                            \
    do {                                        \
        if (TERM_COLOR)                         \
            printf(args);                       \
    } while(0)

char *str_dup(const char *s);
unsigned strsubct(const char *str, const char *sub);
char *strrepl(const char *str, const char *find, const char *repl);
long getline(char **line, size_t *n, FILE *stream);
char *next_tok(char **line);
char *rmqt(const char *str);
char *addqt(const char *str);
void *add_element(void *base, size_t *nmemb, size_t size, unsigned i, void *new_elem);
void remove_element(void *base, size_t *nmemb, size_t size, unsigned i);

