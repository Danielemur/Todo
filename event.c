#include "event.h"
#include "common.h"

void event_init(Event *e,
                Date d, Time t,
                const char *sub,
                const char *loc,
                const char *det)
{
    e->date = d;
    e->time = t;
    e->subject = NULL;
    e->location = NULL;
    e->details = NULL;

    if (sub && *sub)
        e->subject = str_dup(sub);
    if (loc && *loc)
        e->location = str_dup(loc);
    if (det && *det)
        e->details = str_dup(det);
}

void event_destroy(Event *e)
{
    free(e->subject);
    free(e->location);
    free(e->details);
}

void event_print(Event e)
{
    event_fprint(e, stdout);
}

void event_fprint(Event e, FILE *f)
{
    Date d = e.date;
}

void event_snprint(Event e, char *dest, size_t n)
{
    
}

void event_update_date(Event *e, Date d)
{
    e->date = d;
}

void event_update_time(Event *e, Time t)
{
    e->time = t;
}

void event_update_subject(Event *e, const char *sub)
{
    if (e->subject)
        free(e->subject);
    e->subject = str_dup(sub);
}

void event_update_location(Event *e, const char *loc)
{
    if (e->location)
        free(e->location);
    e->location = str_dup(loc);
}

void event_update_details(Event *e, const char *det)
{
    if (e->details)
        free(e->details);
    e->details = str_dup(det);
}

