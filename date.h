#pragma once

#include <stdio.h>

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

void time_print(Time t);
void time_fprint(Time t, FILE *f);
void time_snprint(Time t, char *dest, size_t n);

void date_print(Date d);
void date_fprint(Date d, FILE *f);
void date_snprint(Date d, char *dest, size_t n);

Date date_add_days(Date d1, unsigned days);
