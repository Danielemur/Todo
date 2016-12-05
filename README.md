# Todo
A small command line tool for managing events and tasks, written in C.

___
### Commands
* **DATE**

  Prints out the events for the given date.
* **all**

  Prints all events.
* **date**

  Prints current date.
* **load FILE**

  Loads the events from specified file, prompting if current database has been modified.
* **new**

  Launches an interactive prompt to create a new event.
* **remove, rm DATE [TIME] [INDEX]**

  Removes the event on the given date, or prompts for additional specifiers if multiple events exist.
* **tag TAG**

  Prints out all events in the database which contain the specified tag.
* **save, s**

  Saves the database to its current file location, backing up the existing file.
* **saveas, sa FILE**

  Saves the database to the specified file, backing up existing file.
* **quit, q**

  Exits the program, prompting if database has been modified.

___
### Date format

  Multiple methods of date specification are accepted.
  
  * **MM/DD/YYYY**
  * **today, tomorrow, yesterday**
  * **last DOW**

    Specifies the date from the week prior with the given day of week, e.g. "last Friday".
  * **this DOW**

    Specifies the date from the current week with the given day of week.
  * **next DOW**

    Specifies the date from the following week with the given day of week.
  * **DOW**

    Specifies the date of the next occurance of the given day of week.

___
### CSV Format

Databases are saved and loaded as files in CSV format, conforming to the specifications suggested in [RFC 4180][1].

[1]: https://tools.ietf.org/rfc/rfc4180.txt "RFC 4180"
