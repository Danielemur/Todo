#include "event.h"

#include "common.h"

int TERM_COLOR;

static const char *PRIORITY_TEXT[] = {
    "Low",
    "Medium",
    "High",
    "Urgent"
};

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
    if (e->tags) {
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
        if (TERM_COLOR)
            printf(BOLD GRN);
        date_fprint(e.date, f);
        if (TERM_COLOR)
            printf(RESET);
        fprintf(f, "\n");
    }

    if (flags & PRINT_TIME && time_validate(e.time)) {
        if (TERM_COLOR)
            printf(BOLD MAG);
        time_fprint(e.time, f);
        if (TERM_COLOR)
            printf(RESET);
    }

    if (flags & PRINT_SUBJ && e.subject) {
        if (TERM_COLOR)
            printf(BOLD YEL);
        fprintf(f, "%s\n", e.subject);
        if (TERM_COLOR)
            printf(RESET);
        fprintf(f, "\n");
    }

    if (flags & PRINT_PRTY && priority_validate(e.priority)) {
        if (TERM_COLOR)
            printf(CYN);
        fprintf(f, "Priority:\n");
        if (TERM_COLOR) {
            switch (e.priority) {
            case LOW:
            printf(GRN);
                break;
            case MEDIUM:
            printf(YEL);
                break;
            case HIGH:
            printf(RED);
                break;
            case URGENT:
            printf(BOLD RED);
                break;
            }
        }
        fprintf(f, "%s\n", priority2str(e.priority));
        if (TERM_COLOR)
            printf(RESET);
        fprintf(f, "\n");
    }

    if (flags & PRINT_LCTN && e.location) {
        if (TERM_COLOR)
            printf(CYN);
        fprintf(f, "Location:\n");
        if (TERM_COLOR)
            printf(RESET);
        fprintf(f, "%s\n", e.location);
        fprintf(f, "\n");
    }

    if (flags & PRINT_DTLS && e.details) {
        if (TERM_COLOR)
            printf(CYN);
        fprintf(f, "Details:\n");
        if (TERM_COLOR)
            printf(RESET);
        fprintf(f, "%s\n", e.details);
        fprintf(f, "\n");
    }

    if (flags & PRINT_TAGS && e.tags && e.ntags > 0) {
        if (TERM_COLOR)
            printf(CYN);
        fprintf(f, "Tags:\n");
        if (TERM_COLOR)
            printf(RESET);
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
    if (n > 0)
        fprintf(f, "\n");
    Date last_date = {0};
    for (unsigned i = 0; i < n; i++) {
        uint8_t pflgs = flags;
        if (!date_compare(e[i].date, last_date))
            pflgs &= ~PRINT_DATE;
        else
            last_date = e[i].date;
        event_fprint(e[i], f, pflgs);
    }
}

int event_sort_time(Event e1, Event e2)
{
    int date_cmp = date_compare(e1.date, e2.date);
    return date_cmp ? date_cmp : time_compare(e1.time, e2.time);
}

bool event_equal(Event e1, Event e2)
{
    bool eq = true;
    eq &= !date_compare(e1.date, e2.date);
    eq &= !time_compare(e1.time, e2.time);
    eq &= e1.priority == e2.priority;
    eq &= e1.subject == e2.subject;
    if (e1.subject && e2.subject)
        eq &= !strcmp(e1.subject, e2.subject);
    eq &= e1.location == e2.location;
    if (e1.location && e2.location)
        eq &= !strcmp(e1.location, e2.location);
    eq &= e1.details == e2.details;
    if (e1.details && e2.details)
        eq &= !strcmp(e1.details, e2.details);
    eq &= e1.ntags == e2.ntags;

    if (!eq) {
        return eq;
    } else {
        for (unsigned i = 0; i < e1.ntags; i++) {
            if (strcmp(e1.tags[i], e2.tags[i]))
                return false;
        }
        return true;
    }

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
    if (!strcmp(str, PRIORITY_TEXT[LOW])) {
        return LOW;
    } else if (!strcmp(str, PRIORITY_TEXT[MEDIUM])) {
        return MEDIUM;
    } else if (!strcmp(str, PRIORITY_TEXT[HIGH])) {
        return HIGH;
    } else if (!strcmp(str, PRIORITY_TEXT[URGENT])) {
        return URGENT;
    } else {
        return -1;
    }
}

const char *priority2str(Priority p)
{
    return PRIORITY_TEXT[p];
}

bool priority_validate(Priority p)
{
    return p >= LOW && p <= URGENT;
}
