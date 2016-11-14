#include "event.h"
#include "common.h"

int main(void)
{
    Event e;
    event_init(&e, NULL_DATE, NULL_TIME, -1, NULL, NULL, NULL, NULL, 0);

    event_print(e, PRINT_ALL);

    event_set_date(&e, (Date){2016, 11, 14});
    event_set_time(&e, (Time){4, 0});
    event_set_subject(&e, "Mandatory Advising");
    event_set_location(&e, "Somewhere");
    event_set_details(&e, "Stupid required advising that I must have before I am able to register for classes.");
    event_set_priority(&e, HIGH);

    event_print(e, PRINT_ALL);
    event_print(e, PRINT_ALL - PRINT_PRTY);
    event_print(e, PRINT_ALL - PRINT_DATE);
    event_print(e, PRINT_ALL - PRINT_DTLS);

    event_set_details(&e, NULL);
    event_set_location(&e, "");
    event_set_time(&e, NULL_TIME);

    event_print(e, PRINT_ALL);

    const char *tags[] = {
        "school",
        "dumb",
        "mandatory",
        "annoying"
    };

    event_set_tags(&e, tags, COUNTOF(tags));

    event_print(e, PRINT_ALL);

    event_add_tag(&e, "fish");
    event_add_tag(&e, "taco");

    event_print(e, PRINT_ALL);

    event_remove_tag(&e, "dumb");
    event_remove_tag(&e, "fish");

    event_print(e, PRINT_ALL);

    event_set_tags(&e, NULL, 0);

    event_print(e, PRINT_ALL);

    event_destroy(&e);
}
