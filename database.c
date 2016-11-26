#include "database.h"

#include "common.h"
#include "csv.h"
#include "event.h"

void database_init(Database *db)
{
    db->modified = false;
    db->count = 0;
    db->events = NULL;
}

void database_destroy(Database *db)
{
    for (unsigned i = 0; i < db->count; i++) {
        event_destroy(&db->events[i]);
    }
    db->count = 0;
    free(db->events);
}

static int read_event(Event *e, char *line)
{
    event_init(e, NULL_DATE, NULL_TIME, -1, NULL, NULL, NULL, NULL, 0);
    char *tok;

    if ((tok = csv_next_tok(&line))) {
        event_set_date(e, date_from_str(tok));
        free(tok);
    }
    else return -1;

    if ((tok = csv_next_tok(&line))) {
        event_set_time(e, time_from_str(tok));
        free(tok);
    }
    else return -1;

    if ((tok = csv_next_tok(&line))) {
        event_set_priority(e, priority_from_str(tok));
        free(tok);
    }
    else return -1;

    if ((tok = csv_next_tok(&line))) {
        event_set_subject(e, tok);
        free(tok);
    }
    else return -1;

    if ((tok = csv_next_tok(&line))) {
        event_set_location(e, tok);
        free(tok);
    }
    else return -1;

    if ((tok = csv_next_tok(&line))) {
        event_set_details(e, tok);
        free(tok);
    }
    else return -1;

    while (*line) {
        tok = csv_next_tok(&line);
        if (!tok)
            return -1;
        event_add_tag(e, tok);
        free(tok);
    }

    return 0;
}

int database_load(Database *db, FILE *f)
{
    database_init(db);

    unsigned line_no = 0;
    size_t size = 0;
    char *line = NULL;
    do {
        line_no++;
        if (csv_get_row(&line, &size, f) == -1) {
            if (feof(f)) {
                break;
            } else {
                fprintf(stderr, "Error reading file on line %d!\n", line_no);
                return -1;
            }
        }

        Event e;
        if (read_event(&e, line) != -1) {
            database_add_event(db, e);
        } else {
            fprintf(stderr, "Error reading file on line %d!\n", line_no);
            return -1;
        }
    } while (!feof(f));
    free(line);

    db->modified = false;
    return 0;
}

static unsigned max_tags(Database *db)
{
    unsigned ct = 0;
    for (unsigned i = 0; i < db->count; i++) {
        ct = MAX(ct, db->events[i].ntags);
    }
    return ct;
}

static char *write_event(Event e, char **line, size_t *size, unsigned max_tags)
{
    *size = 0;
    *line = NULL;
    char *empty = "";
    char *tok;

    if (!date_is_null(e.date)) {
        tok = date_to_str(e.date);
        csv_cat_tok(line, size, tok);
        free(tok);
    } else {
        csv_cat_tok(line, size, empty);
    }

    if (!time_is_null(e.time)) {
        tok = time_to_str(e.time);
        csv_cat_tok(line, size, tok);
        free(tok);
    } else {
        csv_cat_tok(line, size, empty);
    }

    if (priority_validate(e.priority)) {
        csv_cat_tok(line, size, priority_to_str(e.priority));
    } else {
        csv_cat_tok(line, size, empty);
    }

    if (e.subject)
        csv_cat_tok(line, size, e.subject);
    else
        csv_cat_tok(line, size, empty);

    if (e.location)
        csv_cat_tok(line, size, e.location);
    else
        csv_cat_tok(line, size, empty);

    if (e.details)
        csv_cat_tok(line, size, e.details);
    else
        csv_cat_tok(line, size, empty);

    for (unsigned i = 0; i < max_tags; i++)
        csv_cat_tok(line, size,
                    (e.tags && i < e.ntags) ? e.tags[i] : empty);

    return *line;
}

int database_save(Database *db, FILE *f)
{
    size_t size;
    char *line;
    unsigned mx_tgs = max_tags(db);
    for (unsigned i = 0; i < db->count; i++) {
        write_event(db->events[i], &line, &size, mx_tgs);
        fprintf(f, "%s\n", line);
    }

    db->modified = false;
    return 0;
}

bool database_is_modified(Database *db)
{
    return db->modified;
}

void database_add_event(Database *db, Event e)
{
    unsigned i;
    for (i = 0; i < db->count && event_sort_time(db->events[i], e) < 0; i++);
    void *new_elem;
    db->events = add_element(db->events, &db->count, sizeof(db->events[0]), i, &new_elem);
    *(Event *)new_elem = e;

    db->modified = true;
}

static int get_event_index(Database *db, Event e)
{
    for (unsigned i = 0; i < db->count; i++) {
        if (event_equal(db->events[i], e))
            return i;
    }
    return -1;
}

void database_remove_event(Database *db, Event e)
{
    int i = get_event_index(db, e);
    if (i >= 0) {
        event_destroy(&db->events[i]);
        remove_element(db->events, &db->count, sizeof(db->events[0]), i);
    }

    db->modified = true;
}

int database_query_date(Database *db, Date d, Event **events, size_t *size)
{
    if (!events || !date_validate(d))
        return -1;

    *events = NULL;
    *size = 0;

    for (unsigned i = 0; i < db->count; i++) {
        if (!date_compare(db->events[i].date, d)) {
            if (!(*events = realloc(*events, ++*size * sizeof((*events)[0]))))
                return -1;
            (*events)[*size - 1] = db->events[i];
        } else if(date_compare(db->events[i].date, d) > 0)
            break;
    }

    return 0;
}

int database_query_date_and_time(Database *db, Date d, Time t, Event **events, size_t *size)
{
    if (!events || !date_validate(d))
        return -1;

    *events = NULL;
    *size = 0;

    for (unsigned i = 0; i < db->count; i++) {
        int dc = date_compare(db->events[i].date, d);
        int tc = time_compare(db->events[i].time, t);
        if (!dc && !tc) {
            if (!(*events = realloc(*events, ++*size * sizeof((*events)[0]))))
                return -1;
            (*events)[*size - 1] = db->events[i];
        } else if((dc == 0 && tc > 0) || dc > 0)
            break;
    }

    return 0;
}

int database_query_tag(Database *db, const char *tag, Event **events, size_t *size)
{
    if (!events || !tag || !*tag)
        return -1;

    *events = NULL;
    *size = 0;

    for (unsigned i = 0; i < db->count; i++) {
        if (event_contains_tag(db->events[i], tag)) {
            if (!(*events = realloc(*events, ++*size * sizeof((*events)[0]))))
                return -1;
            (*events)[*size - 1] = db->events[i];
        }
    }

    return 0;
}
