#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

typedef struct Date {
    unsigned year;
    unsigned month;
    unsigned day;
} Date;

typedef struct Time {
    unsigned hour;
    unsigned minute;
} Time;

static Date NULL_DATE = {-1, -1, -1};
static Time NULL_TIME = {-1, -1};

void  time_print(Time t);
void  time_fprint(Time t, FILE *f);
Time  time_from_str(char *str);
char *time_to_str(Time t);
Time  time_add_minutes(Time t, unsigned minutes);
Time  time_add_hours(Time t, unsigned hours);
int   time_compare(Time t1, Time t2);
bool  time_validate(Time t);
bool  time_is_null(Time t);

void     date_print(Date d);
void     date_fprint(Date d, FILE *f);
Date     date_from_str(char *str);
char    *date_to_str(Date d);
Date     date_add_days(Date d, unsigned days);
Date     date_sub_days(Date d, unsigned days);
int      date_compare(Date d1, Date d2);
bool     date_validate(Date d);
bool     date_is_null(Date d);
unsigned date_day_of_week(Date d);

static int str2dayofweek(char *str)
{
    if (!strcmp(str, "sunday") || !strcmp(str, "Sunday")) {
        return 0;
    } else if (!strcmp(str, "monday") || !strcmp(str, "Monday")) {
        return 1;
    } else if (!strcmp(str, "tuesday") || !strcmp(str, "Tuesday")) {
        return 2;
    } else if (!strcmp(str, "wednesday") || !strcmp(str, "Wednesday")) {
        return 3;
    } else if (!strcmp(str, "thursday") || !strcmp(str, "Thursday")) {
        return 4;
    } else if (!strcmp(str, "friday") || !strcmp(str, "Friday")) {
        return 5;
    } else if (!strcmp(str, "saturday") || !strcmp(str, "Saturday")) {
        return 6;
    } else {
        return -1;
    }
}
