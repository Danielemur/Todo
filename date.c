#include "date.h"

#include <stdbool.h>

#include "common.h"

static const char *MONTH_NAME[] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

static const char *DAY_NAME[] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
};

static const unsigned DAYS_IN_MONTH[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static const char *DATE_SUFFIX[] = {"st", "nd", "rd", "th"};

void time_print(Time t)
{
    time_fprint(t, stdout);
}

void time_fprint(Time t, FILE *f)
{
    if (!time_validate(t))
        fprintf(f, "Invalid time!\n");
    fprintf(f, "%u:%02u %s\n", (t.hour + 11) % 12 + 1, t.minute, t.hour < 11 ? "AM" : "PM");
}

Time time_from_str(char *str)
{
    Time t = {0};
    if (sscanf(str, "%u:%u", &t.hour, &t.minute) == 2)
        return t;
    else
        return NULL_TIME;
}

char *time_to_str(Time t)
{
    if (!time_validate(t))
        return str_dup("Invalid time!");
    else {
        char *ret = malloc(6);
        sprintf(ret, "%02u:%02u", t.hour, t.minute);
        return ret;
    }
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
    if (time_is_null(t1) || time_is_null(t2))
        return time_is_null(t2) - time_is_null(t1);
    return t1.hour != t2.hour ? t1.hour - t2.hour : t1.minute - t2.minute;
}

bool time_validate(Time t)
{
    return t.minute < 60 && t.hour < 24;
}

bool time_is_null(Time t)
{
    return t.hour == -1 && t.minute == -1;
}

void date_print(Date d)
{
    date_fprint(d, stdout);
}

void date_fprint(Date d, FILE *f)
{
    if (!date_validate(d))
        fprintf(f, "Invalid date!\n");
    fprintf(f, "%s, %s %u%s, %u\n",
            DAY_NAME[date_day_of_week(d)],
            MONTH_NAME[d.month - 1],
            d.day,
            DATE_SUFFIX[((d.day - 1) % 10 < 3) && (d.day / 10 != 1) ? (d.day - 1) % 10 : 3],
            d.year);
}

Date date_from_str(char *str)
{
    Date d = {0};
    if (sscanf(str, "%u/%u/%u", &d.month, &d.day, &d.year) == 3)
        return d;
    else
        return NULL_DATE;
}

char *date_to_str(Date d)
{
    if (!date_validate(d))
        return str_dup("Invalid date!");
    else {
        char *ret = malloc(d.year);
        sprintf(ret, "%02u/%02u/%04u", d.month, d.day, d.year);
        return ret;
    }
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
    if (days != 0) {
        d.day += days;
        for (unsigned md = days_in_month(d); d.day > md; md = days_in_month(d)) {
            d.month++;
            if (d.month > 12) {
                d.year++;
                d.month = 1;
            }
            d.day -= md;
        }
    }
    return d;
}

Date date_sub_days(Date d, unsigned days)
{
    while (days >= d.day) {
        d.month--;
        if (d.month <= 0) {
            d.year--;
            d.month = 12;
        }
        days -= d.day;
        d.day = days_in_month(d);
    }
    d.day -= days;
    return d;
}

int date_compare(Date d1, Date d2)
{
    if (date_is_null(d1) || date_is_null(d2))
        return date_is_null(d2) - date_is_null(d1);
    return d1.year != d2.year ? d1.year - d2.year :
           d1.month != d2.month ? d1.month - d2.month : d1.day - d2.day;
}

bool date_validate(Date d)
{
    return d.month > 0 && d.day > 0 &&
        d.month <= 12 && d.day <= days_in_month(d);
}

bool date_is_null(Date d)
{
    return d.year == -1 && d.month == -1 && d.day == -1;
}

//Sakamoto's algorithm
unsigned date_day_of_week(Date date)
{
    unsigned d = date.day;
    unsigned m = date.month;
    unsigned y = date.year;
    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    y -= m < 3;
    return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}
