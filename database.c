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

    if ((date = csv_next_tok(&line)) != NULL)
        event_set_date(e, date_from_str(date));
    else return -1;

    if ((time = csv_next_tok(&line)) != NULL)
        event_set_time(e, time_from_str(time));
    else return -1;

    if ((priority = csv_next_tok(&line)) != NULL)
        event_set_priority(e, str2priority(priority));
    else return -1;

    if ((subject = csv_next_tok(&line)) != NULL)
        event_set_subject(e, subject);
    else return -1;

    if ((location = csv_next_tok(&line)) != NULL)
        event_set_location(e, location);
    else return -1;

    if ((details = csv_next_tok(&line)) != NULL)
        event_set_details(e, details);
    else return -1;

    while (*line) {
        tag = csv_next_tok(&line);
        if (tag == NULL)
            return -1;
        event_add_tag(e, tag);
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
    return 0;
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
    //event_destroy(db->events[i]);
    //remove_element(db->events, &db->count, sizeof(db->events[0]), i);
}

