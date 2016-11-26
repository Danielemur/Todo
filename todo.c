#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>

#include "common.h"
#include "database.h"

/* Error messages */
#define BAD_IN_FRMT_SPEC "%s \"%s\"\n"
static const char *INC_SPEC = "Incomplete specifier";
static const char *UNRC_TOK = "Unrecognised token";
static const char *INV_DATE = "Invalid date";
static const char *INV_TIME = "Invalid time";
static const char *INV_PRTY = "Invalid priority";
static const char *INV_SELN = "Invalid selection";
static const char *BAD_ARG = "Bad argument";
static const char *EXTR_TXT = "Extraneous text";
static const char *RQRS_ARG = "Must provide argument";

/* Boolean for colored output */
extern int TERM_COLOR;

static Date get_current_date()
{
    time_t t = time(NULL);
    struct tm *time = localtime(&t);
    if (time != NULL)
        return (Date){time->tm_year + 1900, time->tm_mon + 1, time->tm_mday};
    else
        return NULL_DATE;
}

static Time get_current_time()
{
    time_t t = time(NULL);
    struct tm *time = localtime(&t);
    if (time != NULL)
        return (Time){time->tm_hour, time->tm_min};
    else
        return NULL_TIME;
}

/* Attempts to read remaining tokens as date. Returns date on success
 * and NULL_DATE on failure. In case of improper formatting, *line set
 * to NULL. Does not increment line if input is not in date format. */
static Date get_date_from_toks(char **line)
{
    Date today = get_current_date();
    Date date = NULL_DATE;
    char *start = *line;
    char *tok = next_tok(&start);

    //if line is empty
    if (!tok)
        return NULL_DATE;

    if (!strcmp(tok, "today")) {
        date = today;
    } else if (!strcmp(tok, "tomorrow")) {
        date = date_add_days(today, 1);
    } else if (!strcmp(tok, "yesterday")) {
        date = date_sub_days(today, 1);
    } else if (!strcmp(tok, "last")) { //date for given day last week
        free(tok);
        tok = next_tok(&start);

        //if nothing follows "last"
        if (!tok) {
            fprintf(stderr, "%s\n", INC_SPEC);
            *line = NULL;
            return NULL_DATE;
        }

        int dow = str2dayofweek(tok);
        //if second token not day of week
        if (dow == -1) {
            fprintf(stderr, BAD_IN_FRMT_SPEC, UNRC_TOK, tok);
            *line = NULL;
        } else {
            date = date_sub_days(today, 7 + date_day_of_week(today) - dow);
        }
    } else if (!strcmp(tok, "next")) { //date for given day next week
        free(tok);
        tok = next_tok(&start);

        //if nothing follows "next"
        if (!tok) {
            fprintf(stderr, "%s\n", INC_SPEC);
            *line = NULL;
            return NULL_DATE;
        }

        int dow = str2dayofweek(tok);
        //if second token not day of week
        if (dow == -1) {
            fprintf(stderr, BAD_IN_FRMT_SPEC, UNRC_TOK, tok);
            *line = NULL;
        } else {
            date = date_add_days(today, 7 + dow - date_day_of_week(today));
        }
    } else if (!strcmp(tok, "this")) { //date for given day this week
        free(tok);
        tok = next_tok(&start);

        //if nothing follows "this"
        if (!tok) {
            fprintf(stderr, "%s\n", INC_SPEC);
            *line = NULL;
            return NULL_DATE;
        }

        int dow = str2dayofweek(tok);
        //if second token not day of week
        if (dow == -1) {
            fprintf(stderr, BAD_IN_FRMT_SPEC, UNRC_TOK, tok);
            *line = NULL;
        } else {
            int diff = dow - date_day_of_week(today);
            if (diff >= 0)
                date = date_add_days(today, diff);
            else
                date = date_sub_days(today, -diff);
        }
    } else if (str2dayofweek(tok) >= 0) { //next occurance of given day
        int offset = str2dayofweek(tok) - date_day_of_week(today);
        offset = offset < 0 ? offset + 7 : offset;
        date = date_add_days(today, offset);
    } else if(!date_is_null(date = date_from_str(tok))) { //numeric date
        if (!date_validate(date)) {
            fprintf(stderr, BAD_IN_FRMT_SPEC, INV_DATE, tok);
            *line = NULL;
            date = NULL_DATE;
        }
    }

    free(tok);

    if (!date_is_null(date))
        *line = start;

    return date;
}

/* Saves database to file, renaming existing file to filepath~ */
static int save(Database *db, char *filepath)
{
    char *backup = malloc(strlen(filepath) + 2);
    strcpy(backup, filepath);
    strcat(backup, "~");

    if (rename(filepath, backup) == -1) {
        if (errno != ENOENT) {
            free(backup);
            return -1;
        }
    }
    free(backup);

    FILE *f = fopen(filepath, "w");
    if (f) {
        if (database_save(db, f) == -1) {
            return -1;
        }
        fclose(f);
    }

    return 0;
}

/* Sets *e to event with given date, time, and index */
static int select_event(Database *db, char **line, Event *e)
{
    char *tok;
    size_t size;
    Event *events;
    int err = -1;
    int which = -1;

    Date d = get_date_from_toks(line);
    if (date_is_null(d)) {
        //print bad arg msg is line doesn't contain date
        if (*line)
            fprintf(stderr, BAD_IN_FRMT_SPEC, BAD_ARG, *line);
        return -1;
    } else if (!**line) {
        //query with date provided
        err = database_query_date(db, d, &events, &size);
    } else {
        tok = next_tok(line);

        Time t = time_from_str(tok);
        if (!time_validate(t)) {
            fprintf(stderr, BAD_IN_FRMT_SPEC, INV_TIME, tok);
        } else {
            //query with date and time provided
            err = database_query_date_and_time(db, d, t, &events, &size);
            if (**line) {
                free(tok);
                tok = next_tok(line);

                //get index for selection from events with same given
                //date and time
                char *endptr;
                which = strtol(tok, &endptr, 10);
                for (; isspace(*endptr) && *endptr; endptr++);

                if (*endptr != '\0' || endptr == tok || which < 0 || which > size - 1) {
                    fprintf(stderr, BAD_IN_FRMT_SPEC, INV_SELN, tok);
                    free(tok);
                    return -1;
                } else if (**line != '\0') {
                    fprintf(stderr, BAD_IN_FRMT_SPEC, EXTR_TXT, *line);
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
                fprintf(stderr, "Multiple events exist, please narrow your selection\n");
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
        } else {
            fprintf(stderr, "No matching events found\n");
            return -1;
        }
    }

    return -1;
}

/* Prompts user for yes, no, or cancel response to msg */
static int get_ync(char *msg)
{
    size_t size = 0;
    char *line = NULL;
    for (;;) {
        if (msg)
            fprintf(stderr, "%s", msg);

        if (getline(&line, &size, stdin) == -1)
            FATAL("Failed to read from stdin!");
        if (!strcmp(line, "y\n") || !strcmp(line, "Y\n")) {
            free(line);
            return 1;
        } else if (!strcmp(line, "n\n") || !strcmp(line, "N\n")) {
            free(line);
            return 0;
        } else if (!strcmp(line, "c\n") || !strcmp(line, "C\n")) {
            free(line);
            return -1;
        } else {
            fprintf(stderr, "Please answer Y/y, N/n, or C/c\n");
            continue;
        }
    }
}

/* Prompts user for data on new event */
static int new_event_prompt(Event *e)
{
    size_t size = 0;
    char *line = NULL;
    char *remaining, *tok;

    event_init(e, NULL_DATE, NULL_TIME, -1, NULL, NULL, NULL, NULL, 0);

    for (;;) {
        if (TERM_COLOR)
            printf(BOLD BLU);
        printf("Please enter a date:\n");
        if (TERM_COLOR)
            printf(RESET);
        if (getline(&line, &size, stdin) == -1)
            FATAL("Failed to read from stdin!");
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';
        remaining = line;
        Date d = get_date_from_toks(&remaining);
        if (date_is_null(d)) {
            if (remaining)
                printf(BAD_IN_FRMT_SPEC, BAD_ARG, remaining);
        } else {
            event_set_date(e, d);
            break;
        }
    }

    for (;;) {
        if (TERM_COLOR)
            printf(BOLD BLU);
        printf("Please enter a time:\n");
        if (TERM_COLOR)
            printf(RESET);
        if (getline(&line, &size, stdin) == -1)
            FATAL("Failed to read from stdin!");
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';
        remaining = line;
        for (; isspace(*remaining) && *remaining; remaining++);
        if (!*remaining)
            break;
        tok = next_tok(&remaining);
        Time t = time_from_str(tok);

        if (*remaining) {
            printf(BAD_IN_FRMT_SPEC, EXTR_TXT, remaining);
            free(tok);
            continue;
        }

        if (!time_validate(t)) {
            printf(BAD_IN_FRMT_SPEC, INV_TIME, tok);
            free(tok);
            continue;
        } else {
            event_set_time(e, t);
            free(tok);
            break;
        }
    }

    for (;;) {
        if (TERM_COLOR)
            printf(BOLD BLU);
        printf("Please enter a Priority (Low, Medium, High, Urgent):\n");
        if (TERM_COLOR)
            printf(RESET);
        if (getline(&line, &size, stdin) == -1)
            FATAL("Failed to read from stdin!");
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';
        remaining = line;
        for (; isspace(*remaining) && *remaining; remaining++);
        if (!*remaining)
            break;
        tok = next_tok(&remaining);
        Priority p = priority_from_str(tok);

        if (*remaining) {
            printf(BAD_IN_FRMT_SPEC, EXTR_TXT, remaining);
            free(tok);
            continue;
        }

        if (!priority_validate(p)) {
            printf(BAD_IN_FRMT_SPEC, INV_PRTY, tok);
            free(tok);
            continue;
        } else {
            event_set_priority(e, p);
            free(tok);
            break;
        }
    }

    for (;;) {
        if (TERM_COLOR)
            printf(BOLD BLU);
        printf("Please enter a subject:\n");
        if (TERM_COLOR)
            printf(RESET);
        if (getline(&line, &size, stdin) == -1)
            FATAL("Failed to read from stdin!");
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';
        remaining = line;
        for (; isspace(*remaining) && *remaining; remaining++);
        if (!*remaining)
            break;
        event_set_subject(e, str_dup(remaining));
        break;
    }

    for (;;) {
        if (TERM_COLOR)
            printf(BOLD BLU);
        printf("Please enter a location:\n");
        if (TERM_COLOR)
            printf(RESET);
        if (getline(&line, &size, stdin) == -1)
            FATAL("Failed to read from stdin!");
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';
        remaining = line;
        for (; isspace(*remaining) && *remaining; remaining++);
        if (!*remaining)
            break;
        event_set_location(e, str_dup(remaining));
        break;
    }

    for (;;) {
        if (TERM_COLOR)
            printf(BOLD BLU);
        printf("Please enter details:\n");
        if (TERM_COLOR)
            printf(RESET);
        if (getline(&line, &size, stdin) == -1)
            FATAL("Failed to read from stdin!");
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';
        remaining = line;
        for (; isspace(*remaining) && *remaining; remaining++);
        if (!*remaining)
            break;
        event_set_details(e, str_dup(remaining));
        break;
    }

    for (;;) {
        if (TERM_COLOR)
            printf(BOLD BLU);
        printf("Please enter a tag, or return if finished:\n");
        if (TERM_COLOR)
            printf(RESET);
        if (getline(&line, &size, stdin) == -1)
            FATAL("Failed to read from stdin!");
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';
        remaining = line;
        for (; isspace(*remaining) && *remaining; remaining++);
        if (!*remaining)
            break;
        event_add_tag(e, str_dup(remaining));
    }

    return 0;
}

static void interactive_mode(Database *db, char **filepath)
{
    char *tok, *remaining, *line = NULL;
    Date d;
    size_t size;
    size_t nevents;
    Event *events;

    for (;;) {
        if (TERM_COLOR)
            printf(BOLD BLU);

        printf("> ");
        fflush(stdout);

        if (getline(&line, &size, stdin) == -1)
            FATAL("Failed to read from stdin!");

        if (TERM_COLOR)
            printf(RESET);

        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';

        remaining = line;

        if (!date_is_null(d = get_date_from_toks(&remaining))) {
            if (*remaining) {
                Time t = time_from_str(remaining);

                if (!time_is_null(t)) {
                    tok = next_tok(&remaining);
                    for (; isspace(*remaining) && *remaining; remaining++);
                    if (*remaining) {
                        fprintf(stderr, BAD_IN_FRMT_SPEC, EXTR_TXT, remaining);
                        free(tok);
                        continue;
                    }
                    if (!time_validate(t)) {
                        fprintf(stderr, BAD_IN_FRMT_SPEC, INV_TIME, tok);
                        free(tok);
                        continue;
                    } else {
                        if (database_query_date_and_time(db, d, t, &events, &nevents) != -1) {
                            event_print_arr(events, nevents, PRINT_ALL);
                            free(events);
                        }
                        free(tok);
                        continue;
                    }
                } else {
                    fprintf(stderr, BAD_IN_FRMT_SPEC, EXTR_TXT, remaining);
                    continue;
                }
            }

            if (database_query_date(db, d, &events, &nevents) != -1) {
                event_print_arr(events, nevents, PRINT_ALL);
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
                fprintf(stderr, BAD_IN_FRMT_SPEC, EXTR_TXT, remaining);
                continue;
            }
            event_print_arr(db->events, db->count, PRINT_ALL);
        } else if (!strcmp(tok, "load")) {
            free(tok);
            tok = next_tok(&remaining);

            if (!tok) {
                fprintf(stderr, "%s\n", RQRS_ARG);
                continue;
            }

            if (*remaining) {
                fprintf(stderr, BAD_IN_FRMT_SPEC, EXTR_TXT, remaining);
                free(tok);
                continue;
            }

            FILE *f = fopen(tok, "r");

            if (!f) {
                fprintf(stderr, "Failed to open file \"%s\"\n", tok);
                free(tok);
                continue;
            }

            Database new_db;
            if (database_load(&new_db, f) == -1) {
                free(tok);
                continue;
            }

            database_destroy(db);
            *db = new_db;
            *filepath = tok;
            fclose(f);

        } else if (!strcmp(tok, "new")) {
            free(tok);

            if (*remaining) {
                fprintf(stderr, BAD_IN_FRMT_SPEC, EXTR_TXT, remaining);
                continue;
            }

            Event e;
            new_event_prompt(&e);
            database_add_event(db, e);
            continue;
        } else if (!strcmp(tok, "remove")) {
            free(tok);

            if (!*remaining) {
                fprintf(stderr, "%s\n", RQRS_ARG);
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
                fprintf(stderr, BAD_IN_FRMT_SPEC, EXTR_TXT, remaining);
                continue;
            }

            if (database_query_tag(db, tok, &events, &nevents) != -1) {
                event_print_arr(events, nevents, PRINT_ALL);
                free(events);
            }

            free(tok);
            continue;
        } else if (!strcmp(tok, "save") || !strcmp(tok, "s")) {
            free(tok);
            if (save(db, *filepath) == -1)
                fprintf(stderr, "Failed to save database\n");
            continue;
        } else if (!strcmp(tok, "saveas") || !strcmp(tok, "sa")) {
            free(tok);
            tok = next_tok(&remaining);

            if (!tok) {
                fprintf(stderr, "%s\n", RQRS_ARG);
                continue;
            }

            if (*remaining) {
                fprintf(stderr, BAD_IN_FRMT_SPEC, EXTR_TXT, remaining);
                free(tok);
                continue;
            }

            if (save(db, tok) == -1) {
                fprintf(stderr, "Failed to save database to file \"%s\"\n", tok);
                free(tok);
            } else {
                *filepath = tok;
            }

            continue;
        } else if (!strcmp(tok, "quit") || !strcmp(tok, "q")) {
            free(tok);
            if (*remaining) {
                fprintf(stderr, BAD_IN_FRMT_SPEC, EXTR_TXT, remaining);
                continue;
            }

            if (database_is_modified(db)) {
                switch (get_ync(
                        "Database has been modified. "
                        "Would you like to save before quitting? (y/n/c) "
                            )) {
                case 1 :
                    if (save(db, *filepath) == -1) {
                        if (get_ync(
                            "Could not save database. "
                            "Would you like to quit anyway? (y/n/c) "
                            ) < 1) {
                            continue;
                        }
                    }
                    break;
                case 0 :
                    break;
                case -1 :
                    continue;
                }
            }

            return;
        } else {
            if (*tok)
                fprintf(stderr, BAD_IN_FRMT_SPEC, UNRC_TOK, tok);
            free(tok);
            continue;
        }
    }
}

static char *get_default_file_path(void)
{
    char *home = getenv("HOME");
    if (!home)
        exit(EXIT_FAILURE);

    char *path = "/.dbtodo";
    char *fullpath = malloc(strlen(home) + strlen(path) + 1);
    strcpy(fullpath, home);
    strcat(fullpath, path);

    return fullpath;
}

int main(int argc, char **argv)
{
    TERM_COLOR = isatty(STDOUT_FILENO);

    bool interactive = false;
    char *filepath = get_default_file_path();
    int option;
    while ((option = getopt(argc, argv, "c:f:i")) != -1) {
        switch (option) {
        case 'c':
            if (optarg[0] == '-') {
                FATAL("%s: option requires an argument -- '%c'", argv[0], option);
            }

            char *endptr;
            TERM_COLOR = strtol(optarg, &endptr, 10);
            if (*endptr != '\0' || endptr == optarg || TERM_COLOR < 0 || TERM_COLOR > 1)
                FATAL(BAD_IN_FRMT_SPEC, INV_SELN, optarg);

            break;
        case 'f':
            free(filepath);
            if (optarg[0] == '-') {
                FATAL("%s: option requires an argument -- '%c'", argv[0], option);
            }
            filepath = str_dup(optarg);
            break;
        case 'i':
            interactive = true;
            break;
        case '?':
            return EXIT_FAILURE;
        }
    }



    FILE *f = fopen(filepath, "r");

    if (!f)
        FATAL("Failed to open file \"%s\"\n", filepath);

    Database db;
    database_load(&db, f);

    fclose(f);

    if (interactive)
        interactive_mode(&db, &filepath);

    free(filepath);
    database_destroy(&db);
    return EXIT_SUCCESS;
}
