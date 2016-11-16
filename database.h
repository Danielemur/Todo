#pragma once

#include <stdlib.h>
#include "event.h"

typedef struct Database {
    size_t count;
    Event *events;
} Database;

void database_init(Database *db);

/* database transfer to/from file */
int database_load(Database *db, FILE *f);
int database_save(Database *db, FILE *f);

void database_add_event(Database *db, Event e);
void database_remove_event(Database *db, Event e);
