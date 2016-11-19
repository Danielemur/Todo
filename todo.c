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

void interactive_mode(Database *db)
{
    char *tok, *remaining, *line = NULL;
    size_t size;
    Event *events;

    for (;;) {
        printf("> ");
        fflush(stdout);

        if (getline(&line, &size, stdin) == -1)
            FATAL("Failed to read from stdin!");

        remaining = line;
        tok = next_tok(&remaining);

        if (!tok) {
            continue;
        } else if (!strcmp(tok, "all")) {
            free(tok);
            event_print_arr(db->events, db->count, PRINT_ALL);
        } else if (!strcmp(tok, "date")) {
            free(tok);
            tok = next_tok(&remaining);

            Date d = date_from_str(tok);
            if (!date_validate(d)) {
                fprintf(stderr, "Invalid date \"%s\"\n", tok);
            } else if (database_query_date(db, d, &events, &size) != -1) {
                event_print_arr(events, size, PRINT_ALL);
                free(events);
            }

            free(tok);
            continue;
        } else if (!strcmp(tok, "remove")) {
            free(tok);
            tok = next_tok(&remaining);

            int err;
            int which = -1;
            Date d = date_from_str(tok);
            if (!date_validate(d)) {
                fprintf(stderr, "Invalid date \"%s\"\n", tok);
            } else if (!*remaining) {
                err = database_query_date(db, d, &events, &size);
            } else {
                free(tok);
                tok = next_tok(&remaining);

                Time t = time_from_str(tok);
                if (!time_validate(t)) {
                    fprintf(stderr, "Invalid time \"%s\"\n", tok);
                } else {
                    err = database_query_date_and_time(db, d, time_from_str(tok), &events, &size);
                    if (*remaining) {
                        free(tok);
                        tok = next_tok(&remaining);
                        
                        char *endptr;
                        which = strtol(tok, &endptr, 10);
                        if (*endptr != '\0' || which < 0) {
                            fprintf(stderr, "Invalid selection \"%s\"\n", tok);
                            free(tok);
                            continue;
                        }
                    }
                }
            }

            if (err != -1) {
                if (size > 1) {
                    if (which == -1 || which >= size) {
                        event_print_arr(events, size, PRINT_ALL);
                        free(events);
                    } else {
                        database_remove_event(db, events[which]);
                    }
                } else if (size == 1) {
                    database_remove_event(db, events[0]);
                }
            }

            free(tok);
            continue;
        } else if (!strcmp(tok, "tag")) {
            free(tok);
            tok = next_tok(&remaining);

            char *noqt = rmqt(tok);
            if (database_query_tag(db, noqt, &events, &size) != -1) {
                event_print_arr(events, size, PRINT_ALL);
                free(events);
            }

            free(tok);
            continue;
        } else if (!strcmp(tok, "today")) {
            free(tok);

            if (database_query_date(db, get_current_date(), &events, &size) != -1) {
                event_print_arr(events, size, PRINT_ALL);
                free(events);
            }

            continue;
        } else if (!strcmp(tok, "tomorrow")) {
            free(tok);

            if (database_query_date(db,
                                    date_add_days(get_current_date(), 1),
                                    &events, &size) != -1) {
                event_print_arr(events, size, PRINT_ALL);
                free(events);
            }

            continue;
        } else if (!strcmp(tok, "yesterday")) {
            free(tok);

            if (database_query_date(db,
                                    date_sub_days(get_current_date(), 1),
                                    &events, &size) != -1) {
                event_print_arr(events, size, PRINT_ALL);
                free(events);
            }

            continue;
        } else if (!strcmp(tok, "quit") || !strcmp(tok, "q")) {
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
