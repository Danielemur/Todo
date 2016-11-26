#pragma once

#include <stdio.h>
#include <inttypes.h>

#include "date.h"

#define PRINT_ALL  0xFF
#define PRINT_DATE 0x01
#define PRINT_TIME 0x02
#define PRINT_PRTY 0x04
#define PRINT_SUBJ 0x08
#define PRINT_LCTN 0x10
#define PRINT_DTLS 0x20
#define PRINT_TAGS 0x40

typedef enum Priority {
    LOW,
    MEDIUM,
    HIGH,
    URGENT
} Priority;

typedef struct Event {
    Date date;
    Time time;
    Priority priority;
    char *subject;
    char *location;
    char *details;
    char **tags;
    size_t ntags;
} Event;

void event_init(Event *e,
                Date d, Time t,
                Priority p,
                const char *sub,
                const char *loc,
                const char *det,
                const char *tags[],
                size_t ntags);
void event_destroy(Event *e);

void event_print(Event e, uint8_t flags);
void event_fprint(Event e, FILE *f, uint8_t flags);
void event_print_arr(Event *e, size_t n, uint8_t flags);
void event_fprint_arr(Event *e, size_t n, FILE *f, uint8_t flags);

int  event_sort_time(Event e1, Event e2);
bool event_equal(Event e1, Event e2);

void event_set_date(Event *e, Date d);
void event_set_time(Event *e, Time t);
void event_set_priority(Event *e, Priority p);
void event_set_subject(Event *e, const char *sub);
void event_set_location(Event *e, const char *loc);
void event_set_details(Event *e, const char *det);
void event_set_tags(Event *e, const char *tags[], size_t ntags);
void event_add_tag(Event *e, const char *tag);
void event_remove_tag(Event *e, const char *tag);
bool event_contains_tag(Event e, const char *tag);

Priority priority_from_str(char *str);
const char *priority_to_str(Priority p);
bool priority_validate(Priority p);
