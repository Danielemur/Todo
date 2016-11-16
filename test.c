#include <stdlib.h>

#include "event.h"
#include "common.h"
#include "database.h"

int main(int argc, char **argv)
{
    if (argc <= 1)
        return EXIT_FAILURE;
    FILE *f = fopen(argv[1], "r");
    if (!f)
        FATAL("Failed to open file \"%s\"\n", argv[1]);

    Database db;
    if (database_load(&db, f) != -1) {
        event_print_arr(db.events, db.count, PRINT_ALL);
    }
}
