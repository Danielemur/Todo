#include "event.h"
#include "common.h"

static bool priority_validate(Priority p)
{
    return p >= LOW && p <= URGENT;
}

static int strcmp_wrapper(const void *a, const void *b)
{
    return strcmp(*((char **)a), *((char **)b));
}

static void cpy_tags(Event *e, const char *tags[], size_t ntags)
{
    e->ntags = ntags;
    e->tags = malloc(e->ntags * sizeof(e->tags[0]));
    for (unsigned i = 0; i < e->ntags; i++) {
        e->tags[i] = str_dup(tags[i]);
    }
    qsort(e->tags, e->ntags, sizeof(e->tags[0]), strcmp_wrapper);
}

static void free_tags(Event *e)
{
    if (e->tags != NULL) {
        for (unsigned i = 0; i < e->ntags; i++)
            free(e->tags[i]);
        free(e->tags);
        e->ntags = 0;
    }
}

void event_init(Event *e,
                Date d, Time t,
                Priority p,
                const char *sub,
                const char *loc,
                const char *det,
                const char *tags[],
                size_t ntags)
{
    e->date = d;
    e->time = t;
    e->priority = priority_validate(p) ? p : -1;
    e->subject = NULL;
    e->location = NULL;
    e->details = NULL;
    e->tags = NULL;
    e->ntags = 0;

    if (sub && *sub)
        e->subject = str_dup(sub);
    if (loc && *loc)
        e->location = str_dup(loc);
    if (det && *det)
        e->details = str_dup(det);
    if (tags && ntags > 0)
        cpy_tags(e, tags, ntags);
}

void event_destroy(Event *e)
{
    free(e->subject);
    e->subject = NULL;
    free(e->location);
    e->location = NULL;
    free(e->details);
    e->details = NULL;
    free_tags(e);
    e->tags = NULL;
}

void event_print(Event e, uint8_t flags)
{
    event_fprint(e, stdout, flags);
}

void event_fprint(Event e, FILE *f, uint8_t flags)
{

    if (flags & PRINT_DATE && date_validate(e.date)) {
        fprintf(f, "\n");
        date_fprint(e.date, f);
        fprintf(f, "\n");
    }

    if (flags & PRINT_TIME && time_validate(e.time))
        time_fprint(e.time, f);

    if (flags & PRINT_SUBJ && e.subject) {
        fprintf(f, "%s\n", e.subject);
        fprintf(f, "\n");
    }

    if (flags & PRINT_PRTY && priority_validate(e.priority)) {
        fprintf(f, "Priority:\n");
        fprintf(f, "%s\n", PRIORITY_TEXT[e.priority]);
        fprintf(f, "\n");
    }

    if (flags & PRINT_LCTN && e.location) {
        fprintf(f, "Location:\n");
        fprintf(f, "%s\n", e.location);
        fprintf(f, "\n");
    }

    if (flags & PRINT_DTLS && e.details) {
        fprintf(f, "Details:\n");
        fprintf(f, "%s\n", e.details);
        fprintf(f, "\n");
    }

    if (flags & PRINT_TAGS && e.tags && e.ntags > 0) {
        fprintf(f, "Tags:\n");
        for (unsigned i = 0; i < e.ntags; i++)
            fprintf(f, (i != e.ntags - 1) ? "%s, " : "%s\n", e.tags[i]);
        fprintf(f, "\n");
    }

    fprintf(f, "\n");
}

void event_print_arr(Event *e, size_t n, uint8_t flags)
{
    event_fprint_arr(e, n, stdout, flags);
}

void event_fprint_arr(Event *e, size_t n, FILE *f, uint8_t flags)
{
    Date last_date = {0};
    for (unsigned i = 0; i < n; i++) {
        if (!date_compare(e[i].date, last_date))
            flags &= ~PRINT_DATE;
        else
            last_date = e[i].date;
        event_fprint(e[i], f, flags);
    }
}

int event_sort_time(Event e1, Event e2)
{
    int date_cmp = date_compare(e1.date, e2.date);
    return date_cmp ? date_cmp : time_compare(e1.time, e2.time);
}

void event_set_date(Event *e, Date d)
{
    if (!date_validate(d))
        e->date = NULL_DATE;
    else
        e->date = d;
}

void event_set_time(Event *e, Time t)
{
    if (!time_validate(t))
        e->time = NULL_TIME;
    else
        e->time = t;
}

void event_set_priority(Event *e, Priority p)
{
    e->priority = -1;
    if (priority_validate(p))
        e->priority = p;
}

void event_set_subject(Event *e, const char *sub)
{
    if (e->subject)
        free(e->subject);
    e->subject = NULL;

    if (sub && *sub)
        e->subject = str_dup(sub);
}

void event_set_location(Event *e, const char *loc)
{
    if (e->location)
        free(e->location);
    e->location = NULL;

    if (loc && *loc)
        e->location = str_dup(loc);
}

void event_set_details(Event *e, const char *det)
{
    if (e->details)
        free(e->details);
    e->details = NULL;

    if (det && *det)
        e->details = str_dup(det);
}

void event_set_tags(Event *e, const char *tags[], size_t ntags)
{
    if (e->tags && e->ntags > 0)
        free(tags);
    e->tags = NULL;

    if (tags && ntags > 0)
        cpy_tags(e, tags, ntags);
}

void event_add_tag(Event *e, const char *tag)
{
    if (tag && *tag) {
        unsigned i;
        for (i = 0; i < e->ntags && strcmp(e->tags[i], tag) < 0; i++);
        if (i == e->ntags || strcmp(e->tags[i], tag)) {
            void *new_elem;
            e->tags = add_element(e->tags, &e->ntags, sizeof(e->tags[0]), i, &new_elem);
            *(char **)new_elem = str_dup(tag);
        }
    }
}

static int get_tag_index(Event e, const char *tag)
{
    int start = 0;
    size_t size = e.ntags;

    while (size >= 1) {
        char **offset = &e.tags[start + (size / 2)];
        if (!strcmp(*offset, tag))
            return start + (size / 2);
        bool lt = strcmp(*offset, tag) > 0;
        start = lt ? start : start + (size / 2);
        size = lt ? size / 2 : size - ((size + 1) / 2);
    }

    return -1;
}

void event_remove_tag(Event *e, const char *tag)
{
    int tag_ind;
    if ((tag_ind = get_tag_index(*e, tag)) >= 0) {
        free(e->tags[tag_ind]);
        remove_element(e->tags, &e->ntags, sizeof(e->tags[0]), tag_ind);
    }
}

bool event_contains_tag(Event e, const char *tag)
{
    return get_tag_index(e, tag) != -1;
}

Priority str2priority(char *str)
{
    if (!strcmp(str, "low")) {
        return LOW;
    } else if (!strcmp(str, "med")) {
        return MEDIUM;
    } else if (!strcmp(str, "high")) {
        return HIGH;
    } else if (!strcmp(str, "urgent")) {
        return URGENT;
    } else {
        return -1;
    }
}
