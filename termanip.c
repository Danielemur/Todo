#include "termanip.h"

#include <termios.h>
#include <ctype.h>

#include "common.h"

static struct termios old, new;

void start_noncannon(void)
{
    tcgetattr(0, &old);
    new = old;
    new.c_lflag &= ~ICANON;
    new.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &new);
}

void end_noncannon(void)
{
    tcsetattr(0, TCSANOW, &old);
}

void set_cursor_pos(vec2 p)
{
    printf("\033[%u;%uH", p.y, p.x);
}

int try_read_pos(char *resp, size_t *n)
{
    char c;
    while ((c = getchar()) != '\033');
    if ((c = getchar()) != '[')
        return 0;
    while ((c = getchar()) != ';') {
        if (!isdigit(c))
            return 0;
        else {
            resp[(*n)++] = c;
            if (*n > 64)
                FATAL("Critical Error!");
        }
    }

    resp[(*n)++] = ';';
    if (*n > 64)
        FATAL("Critical Error!");

    while ((c = getchar()) != 'R') {
        if (!isdigit(c))
            return 0;
        else {
            resp[(*n)++] = c;
            if (*n > 64)
                FATAL("Critical Error!");
        }
    }
    resp[*n] = '\0';
    return 1;
}

vec2 get_cursor_pos(void)
{
    vec2 p;
    size_t n = 0;
    char resp[65] = "";
    printf("\033[6n");
    while (!try_read_pos(resp, &n));
    sscanf(resp, "%u;%u", &p.y, &p.x);
    return p;
}

vec2 get_term_size(void)
{
    vec2 s;
    size_t n = 0;
    char resp[65] = "";
    printf("\033[s\033[999;999H\033[6n\033[u");
    while (!try_read_pos(resp, &n));
    sscanf(resp, "%u;%u", &s.y, &s.x);
    return s;
}
