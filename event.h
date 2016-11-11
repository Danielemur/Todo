#pragma once

#include <stdio.h>

#include "date.h"

typedef struct Event {
    Date date;
    Time time;
    char *subject;
    char *location;
    char *details;
} Event;

void event_init(Event *e,
                Date d, Time t,
                const char *sub,
                const char *loc,
                const char *det);
void event_destroy(Event *e);

void event_print(Event e);
void event_fprint(Event e, FILE *f);
void event_snprint(Event e, char *dest, size_t n);

void event_update_date(Event *e, Date d);
void event_update_time(Event *e, Time t);
void event_update_subject(Event *e, const char *sub);
void event_update_location(Event *e, const char *loc);
void event_update_details(Event *e, const char *det);
