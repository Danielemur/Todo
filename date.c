#include "date.h"

#include <stdbool.h>

//Sakamoto's algorithm
static unsigned day_of_week(unsigned d, unsigned m, unsigned y)
{
    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    y -= m < 3;
    return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}

void time_print(Time t)
{
    time_fprint(t, stdout);
}

void time_fprint(Time t, FILE *f)
{
    fprintf(f, "%d:%02d %s\n", (t.hour + 11) % 12 + 1, t.minute, t.hour < 11 ? "AM" : "PM");
}

void time_snprint(Time t, char *dest, size_t n)
{
    snprintf(dest, n, "%d:%20d %s\n", (t.hour + 11) % 12 + 1, t.minute, t.hour < 11 ? "AM" : "PM");
}

void date_print(Date d)
{
    date_fprint(d, stdout);
}

void date_fprint(Date d, FILE *f)
{
    fprintf(f, "%s, %s %d%s, %d\n",
            DAY_NAME[day_of_week(d.day, d.month, d.year)],
            MONTH_NAME[d.month - 1],
            d.day,
            DATE_SUFFIX[((d.day - 1) % 10 < 3) && (d.day / 10 != 1) ? (d.day - 1) % 10 : 3],
            d.year);
}

void date_snprint(Date d, char *dest, size_t n)
{
    snprintf(dest, n, "%s, %s %d%s, %d\n",
             DAY_NAME[day_of_week(d.day, d.month, d.year)],
             MONTH_NAME[d.month - 1],
             d.day,
             DATE_SUFFIX[((d.day - 1) % 10 < 3) && (d.day / 10 != 1) ? (d.day - 1) % 10 : 3],
             d.year);
}

static bool leapday(unsigned y)
{
    return (!(y % 4) && y % 100) || !(y % 400);
}

static unsigned days_in_month(Date d)
{
    unsigned retval = DAYS_IN_MONTH[d.month - 1];
    
    if (d.month == 2 && leapday(d.year))
        retval++;
    return retval;
}

Date date_add_days(Date d, unsigned days)
{
    d.day += days;
    for (int md = days_in_month(d); d.day > md; md = days_in_month(d)) {
        d.month++;
        if (d.month > 12) {
            d.year++;
            d.month = 1;
        }
        d.day -= md;
    }

    return d;
}

