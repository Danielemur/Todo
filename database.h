#pragma once

typedef struct Database {
    
} Database;

/* database transfer to/from file */
int database_load(Database *db, const char *filename);
int database_save(Database *db, const char *filename);

void database_add_event(Database *db, Event e);
void database_remove_event(Database *db, Event e);
