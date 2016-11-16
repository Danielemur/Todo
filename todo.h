#pragma once

typedef struct Date {
    unsigned year;
    unsigned month;
    unsigned day;
} Date;

typedef struct Time {
    unsigned hour;
    unsigned minute;
} Time;

typedef struct Event {
    Date date;
    Time time;
    char *subject;
    char *location;
    char *details;
} Event;

typedef struct Database {

} Database;

/* database transfer to/from file */
int database_load(Database *db, const char *filename);
int database_save(Database *db, const char *filename);
