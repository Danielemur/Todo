#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "database.h"

static Date get_current_date()
{
    time_t t = time(NULL);
    struct tm *time = localtime(&t);
    return (Date){time->tm_year + 1900, time->tm_mon + 1, time->tm_mday};
}

static Time get_current_time()
{
    time_t t = time(NULL);
    struct tm *time = localtime(&t);
    return (Time){time->tm_hour, time->tm_min};
}

static Date get_date_from_toks(char **line)
{
    Date today = get_current_date();
    Date date = NULL_DATE;
    char *start = *line;
    char *tok = next_tok(&start);

    if (!strcmp(tok, "today")) {
        date = today;
    } else if (!strcmp(tok, "tomorrow")) {
        date = date_add_days(today, 1);
    } else if (!strcmp(tok, "yesterday")) {
        date = date_sub_days(today, 1);
    } else if (!strcmp(tok, "last")) {
        free(tok);
        tok = next_tok(&start);

        int dow = str2dayofweek(tok);
        if (dow == -1) {
            fprintf(stderr, "Unrecognised token \"%s\"\n", tok);
            *line = NULL;
        } else {
            date = date_sub_days(today, 7 + date_day_of_week(today) - dow);
        }
    } else if (!strcmp(tok, "next")) {
        free(tok);
        tok = next_tok(&start);

        int dow = str2dayofweek(tok);
        if (dow == -1) {
            fprintf(stderr, "Unrecognised token \"%s\"\n", tok);
            *line = NULL;
        } else {
            date = date_add_days(today, 7 + dow - date_day_of_week(today));
        }
    } else if (!strcmp(tok, "this")) {
        free(tok);
        tok = next_tok(&start);

        int dow = str2dayofweek(tok);
        if (dow == -1) {
            fprintf(stderr, "Unrecognised token \"%s\"\n", tok);
            *line = NULL;
        } else {
            int diff = dow - date_day_of_week(today);
            if (diff >= 0)
                date = date_add_days(today, diff);
            else
                date = date_sub_days(today, -diff);
        }
    } else if (str2dayofweek(tok) >= 0) {
        int offset = str2dayofweek(tok) - date_day_of_week(today);
        offset = offset < 0 ? offset + 7 : offset;
        date = date_add_days(today, offset);
    } else if(!date_is_null(date = date_from_str(tok))) {
        if (!date_validate(date)) {
            fprintf(stderr, "Invalid date \"%s\"\n", tok);
            *line = NULL;
            date = NULL_DATE;
        }
    }

    free(tok);

    if (!date_is_null(date))
        *line = start;

    return date;
}

int select_event(Database *db, char **line, Event *e)
{
    char *tok;
    size_t size;
    Event *events;
    int err = -1;
    int which = -1;

    Date d = get_date_from_toks(line);
    if (date_is_null(d)) {
        if (*line)
            fprintf(stderr, "Bad argument \"%s\"\n", *line);
        return -1;
    } else if (!**line) {
        err = database_query_date(db, d, &events, &size);
    } else {
        tok = next_tok(line);

        Time t = time_from_str(tok);
        if (!time_validate(t)) {
            fprintf(stderr, "Invalid time \"%s\"\n", tok);
        } else {
            err = database_query_date_and_time(db, d, time_from_str(tok), &events, &size);
            if (**line) {
                free(tok);
                tok = next_tok(line);

                char *endptr;
                which = strtol(tok, &endptr, 10);
                for (; isspace(*endptr) && *endptr; (*endptr)++);

                if (*endptr != '\0' || endptr == tok || which < 0 || which > size - 1) {
                    fprintf(stderr, "Invalid selection \"%s\"\n", tok);
                    free(tok);
                    return -1;
                }

                free(tok);
            }
        }
    }

    if (err != -1) {
        if (size > 1) {
            if (which == -1) {
                printf("Multiple events exist, please narrow your selection\n");
                event_print_arr(events, size, PRINT_ALL);
                free(events);
                return -1;
            } else {
                *e = events[which];
                free(events);
                return 0;
            }
        } else if (size == 1) {
            *e = events[0];
            free(events);
            return 0;
        }
    }

    return -1;
}

void interactive_mode(Database *db)
{
    char *tok, *remaining, *line = NULL;
    Date d;
    size_t size;
    Event *events;

    for (;;) {
        printf("> ");
        fflush(stdout);

        if (getline(&line, &size, stdin) == -1)
            FATAL("Failed to read from stdin!");

        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';

        remaining = line;

        if (!date_is_null(d = get_date_from_toks(&remaining))) {
            if (*remaining) {
                Time t = time_from_str(remaining);

                if (!time_is_null(t)) {
                    if (!time_validate(t)) {
                        fprintf(stderr, "Invalid time \"%s\"\n", tok);
                        continue;
                    } else {
                        if (database_query_date_and_time(db, d, t, &events, &size) != -1) {
                            event_print_arr(events, size, PRINT_ALL);
                            free(events);
                        }
                        continue;
                    }
                } else {
                    fprintf(stderr, "Extraneous text \"%s\"\n", remaining);
                    continue;
                }
            }

            if (database_query_date(db, d, &events, &size) != -1) {
                event_print_arr(events, size, PRINT_ALL);
                free(events);
            }

            continue;
        } else if (!remaining) {
            continue;
        }

        tok = next_tok(&remaining);

        if (!tok) {
            continue;
        } else if (!strcmp(tok, "all")) {
            free(tok);
            if (*remaining) {
                fprintf(stderr, "Extraneous text \"%s\"\n", remaining);
                continue;
            }
            event_print_arr(db->events, db->count, PRINT_ALL);
        } else if (!strcmp(tok, "remove")) {
            free(tok);

            if (!*remaining) {
                fprintf(stderr, "Must provide argument\n");
                continue;
            }

            Event e;
            if (select_event(db, &remaining, &e) != -1)
                database_remove_event(db, e);

            continue;
        } else if (!strcmp(tok, "tag")) {
            free(tok);
            tok = next_tok(&remaining);

            if (*remaining) {
                fprintf(stderr, "Extraneous text \"%s\"\n", remaining);
                continue;
            }

            char *noqt = rmqt(tok);
            if (database_query_tag(db, noqt, &events, &size) != -1) {
                event_print_arr(events, size, PRINT_ALL);
                free(events);
            }

            free(tok);
            continue;
        } else if (!strcmp(tok, "quit") || !strcmp(tok, "q")) {
            if (*remaining) {
                fprintf(stderr, "Extraneous text \"%s\"\n", remaining);
                continue;
            }
            free(tok);
            exit(EXIT_SUCCESS);
        } else {
            if (*tok)
                fprintf(stderr, "Unrecognised token \"%s\"\n", tok);
            free(tok);
            continue;
        }
    }
}

static FILE *get_default_file(void)
{
    char *home = getenv("HOME");
    if (!home)
        exit(EXIT_FAILURE);

    char *path = "/.dbtodo";
    char *fullpath = malloc(strlen(home) + strlen(path) + 1);
    strcpy(fullpath, home);
    strcat(fullpath, path);

    FILE *f = fopen(fullpath, "r");
    free(fullpath);
    if (!f)
        exit(EXIT_FAILURE);
    else
        return f;
}

int main(int argc, char **argv)
{

    FILE *f = get_default_file();

    /* if (!f) */
    /*     FATAL("Failed to open file \"%s\"\n", argv[1]); */

    Database db;
    database_load(&db, f);

    int option;
    while ((option = getopt(argc, argv, "fi")) != -1) {
        switch (option) {
        case 'f':
            break;
        case 'i':
            interactive_mode(&db);
            break;
        case '?':
            break;
        }
    }
}
