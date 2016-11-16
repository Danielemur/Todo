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
    if (!time_validate(t))
        fprintf(f, "Invalid time!\n");
    fprintf(f, "%d:%02d %s\n", (t.hour + 11) % 12 + 1, t.minute, t.hour < 11 ? "AM" : "PM");
}

Time time_from_str(char *str)
{
    Time t = {0};
    sscanf(str, "%d:%d", &t.hour, &t.minute);
    return t;
}

Time time_add_minutes(Time t, unsigned minutes)
{
    t.minute += minutes;
    while (t.minute > 59) {
        t.hour++;
        if (t.hour > 23)
            t.hour -= 24;
        t.minute -= 60;
    }
    return t;
}

Time time_add_hours(Time t, unsigned hours)
{
    t.hour += hours;
    while (t.hour > 24)
        t.hour -= 24;
    return t;
}

int time_compare(Time t1, Time t2)
{

    return t1.hour != t2.hour ? t1.hour - t2.hour : t1.minute - t2.minute;
}

bool time_validate(Time t)
{
    return t.minute <= 60 && t.hour <= 24;
}

void date_print(Date d)
{
    date_fprint(d, stdout);
}

void date_fprint(Date d, FILE *f)
{
    if (!date_validate(d))
        fprintf(f, "Invalid date!\n");
    fprintf(f, "%s, %s %d%s, %d\n",
            DAY_NAME[day_of_week(d.day, d.month, d.year)],
            MONTH_NAME[d.month - 1],
            d.day,
            DATE_SUFFIX[((d.day - 1) % 10 < 3) && (d.day / 10 != 1) ? (d.day - 1) % 10 : 3],
            d.year);
}

Date date_from_str(char *str)
{
    Date d = {0};
    sscanf(str, "%d/%d/%d", &d.month, &d.day, &d.year);
    return d;
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
    for (unsigned md = days_in_month(d); d.day > md; md = days_in_month(d)) {
        d.month++;
        if (d.month > 12) {
            d.year++;
            d.month = 1;
        }
        d.day -= md;
    }
    return d;
}

int date_compare(Date d1, Date d2)
{

    return d1.year != d2.year ? d1.year - d2.year :
           d1.month != d1.month ? d1.month - d2.month : d1.day - d2.day;
}

bool date_validate(Date d)
{
    return d.month <= 12 && d.day <= days_in_month(d);
}
