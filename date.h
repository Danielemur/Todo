#pragma once

#include <stdio.h>
#include <stdbool.h>

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

void time_print(Time t);
void time_fprint(Time t, FILE *f);

bool time_validate(Time t);

void date_print(Date d);
void date_fprint(Date d, FILE *f);

Date date_add_days(Date d1, unsigned days);

bool date_validate(Date d);
