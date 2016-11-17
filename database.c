#include "database.h"

#include "common.h"
#include "csv.h"

void database_init(Database *db)
{
    db->count = 0;
    db->events = NULL;
}

static int read_event(Event *e, char *line)
{
    event_init(e, NULL_DATE, NULL_TIME, -1, NULL, NULL, NULL, NULL, 0);
    char *date, *time, *priority, *subject, *location, *details, *tag;

    if ((date = csv_next_tok(&line)) != NULL) {
        event_set_date(e, date_from_str(date));
        free(date);
    }
    else return -1;

    if ((time = csv_next_tok(&line)) != NULL) {
        event_set_time(e, time_from_str(time));
        free(time);
    }
    else return -1;

    if ((priority = csv_next_tok(&line)) != NULL) {
        event_set_priority(e, str2priority(priority));
        free(priority);
    }
    else return -1;

    if ((subject = csv_next_tok(&line)) != NULL) {
        event_set_subject(e, subject);
        free(subject);
    }
    else return -1;

    if ((location = csv_next_tok(&line)) != NULL) {
        event_set_location(e, location);
        free(location);
    }
    else return -1;

    if ((details = csv_next_tok(&line)) != NULL) {
        event_set_details(e, details);
        free(details);
    }
    else return -1;

    while (*line) {
        tag = csv_next_tok(&line);
        if (tag == NULL)
            return -1;
        event_add_tag(e, tag);
        free(tag);
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
        if (csv_get_line(&line, &size, f) == -1) {
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
    return 0;
}

int database_save(Database *db, FILE *f)
{
    return 0; //todo
}

void database_add_event(Database *db, Event e)
{
    unsigned i;
    for (i = 0; i < db->count && event_sort_time(db->events[i], e) < 0; i++);
    void *new_elem;
    db->events = add_element(db->events, &db->count, sizeof(db->events[0]), i, &new_elem);
    *(Event *)new_elem = e;
}

void database_remove_event(Database *db, Event e)
{
    //todo
    //event_destroy(db->events[i]);
    //remove_element(db->events, &db->count, sizeof(db->events[0]), i);
}

int database_query_date(Database *db, Date date, Event **events, size_t *size)
{
    if (!events || !date_validate(date))
        return -1;

    *events = NULL;
    *size = 0;

    for (unsigned i = 0; i < db->count; i++) {
        if (!date_compare(db->events[i].date, date)) {
            if ((*events = realloc(*events, ++*size * sizeof((*events)[0]))) == NULL)
                return -1;
            (*events)[*size - 1] = db->events[i];
        } else if(date_compare(db->events[i].date, date) > 0)
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
            if ((*events = realloc(*events, ++*size * sizeof((*events)[0]))) == NULL)
                return -1;
            (*events)[*size - 1] = db->events[i];
        }
    }

    return 0;
}
