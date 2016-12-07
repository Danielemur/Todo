#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define FATAL(args...)                          \
    do {                                        \
        fprintf(stderr, args);                  \
        exit(EXIT_FAILURE);                     \
    } while(0)

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

typedef struct vec2 {
    unsigned x, y;
} vec2;

static inline void vec2_print(vec2 v)
{
    printf("(%u, %u)", v.x, v.y);
}

static inline vec2 vec2_add(vec2 v1, vec2 v2)
{
    return (vec2){v1.x + v2.x, v1.y + v2.y};
}

void set_cursor_pos(vec2 p)
{
    printf("\033[%u;%uH", p.y, p.x);
}

static int try_read_pos(char *resp, size_t *n)
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

static bool OVWRT = false;

static inline void ensure_size(char **str, size_t *n)
{
    if (strlen(*str) + 2 > *n) {
        *n *= 2;
        *str = realloc(*str, *n);
    }
}

void insert_char(char **str, size_t *n, char c, unsigned *i)
{
    if (OVWRT) {
        if (*i == strlen(*str)) {
            ensure_size(str, n);
            (*str)[*i + 1] = '\0';
        }
        (*str)[*i] = c;
    } else {
        ensure_size(str, n);
        char *offset = *str + *i;
        memmove(offset + 1, offset, strlen(offset) + 1);
        (*str)[*i] = c;
    }
    (*i)++;
}

void remove_char(char **str, size_t *n, unsigned i)
{
    if (strlen(*str) < *n / 2) {
        *n /= 2;
        *str = realloc(*str, *n);
    }
    char *offset = *str + i + 1;
    memmove(offset - 1, offset, strlen(offset) + 1);
}

static inline void backward_delete_char(char **str, size_t *n, unsigned *i)
{
    remove_char(str, n, --*i);
}

static inline void forward_delete_char(char **str, size_t *n, unsigned *i)
{
    remove_char(str, n, *i);
}

static inline void forward_word(char *str, int *index)
{
    for (; str[*index] && !isalnum(str[*index]); (*index)++);
    for (; str[*index] && isalnum(str[*index]); (*index)++);
}

static inline void backward_word(char *str, int *index)
{
    for (; *index > 0 && !isalnum(str[*index - 1]); (*index)--);
    for (; *index > 0 && isalnum(str[*index - 1]); (*index)--);
    if (!isalnum(str[*index]) && *index > 0)
        (*index)++;
}

static inline void up_line(char *str, int *index, vec2 dim)
{
    (void)str;
    if (*index - (int)dim.x >= 0)
        *index -= dim.x;
}

static inline void down_line(char *str, int *index, vec2 dim)
{
    if (*index + dim.x - 1 < strlen(str))
        *index += dim.x;
    else
        *index = strlen(str);
}

void forward_delete_word(char **str, size_t *n, unsigned *i)
{
    unsigned end = *i;
    forward_word(*str, &end);
    unsigned diff = end - *i;

    if (strlen(*str) + 1 - diff < *n / 2) {
        *n /= 2;
        *str = realloc(*str, *n);
    }

    char *offset = *str + *i + diff;
    memmove(offset - diff, offset, strlen(offset) + 1);
}

void backward_delete_word(char **str, size_t *n, unsigned *i)
{
    unsigned end = *i;
    backward_word(*str, &end);
    unsigned diff = *i - end;

    if (strlen(*str) + 1 - diff < *n / 2) {
        *n /= 2;
        *str = realloc(*str, *n);
    }

    char *offset = *str + end + diff;
    memmove(offset - diff, offset, strlen(offset) + 1);
    *i = end;
}

static void add_wrap(vec2 *pos, vec2 dim, unsigned n)
{
    pos->y += (pos->x + n - 1) / dim.x;
    pos->x += n;
    pos->x = ((pos->x - 1) % dim.x) + 1;
}

static int get_pos_from_index(char *str, vec2 start, vec2 dim,
                              unsigned index, vec2 *pos)
{
    *pos = (vec2){start.x, start.y};
    add_wrap(pos, dim, index);
}

void do_ctrl(char c, vec2 dim, vec2 start, vec2 curr,
             char **new_str, size_t *n, int *index)
{
    unsigned len = strlen(*new_str);
    if(c == '\177') { //backspace
        if (*index > 0)
            backward_delete_char(new_str, n, index);
    } else if(c == '\001') { //beginning of line
        *index = 0;
    } else if(c == '\002') { //left
        if (*index - 1 >= 0)
            (*index)--;
    } else if(c == '\005') { //end of line
        *index = strlen(*new_str);
    } else if(c == '\006') { //right
        if (*index < len)
            (*index)++;
    } else if(c == '\010') { //backspace word
        backward_delete_word(new_str, n, index);
    } else if(c == '\016') { //down
        down_line(*new_str, index, dim);
    } else if(c == '\020') { //up
        up_line(*new_str, index, dim);
    } else if(c == '\033') {
        if ((c = getchar()) == '[') {
            switch (c = getchar()) {
            case 'A': //up
                up_line(*new_str, index, dim);
                break;
            case 'B': //down
                down_line(*new_str, index, dim);
                break;
            case 'C': //right
                if (*index < len)
                    (*index)++;
                break;
            case 'D': //left
                if (*index - 1 >= 0)
                    (*index)--;
                break;
            case '2':
                OVWRT = !OVWRT;
                break;
            case '3':
                switch (getchar()) {
                case  '~':
                    if (*index <= len) {
                        forward_delete_char(new_str, n, index);
                    }
                    break;
                case ';':
                    if (getchar() == '5') {
                        if (getchar() == '~') {
                            forward_delete_word(new_str, n, index);
                        }
                    }
                    break;
                }
                    break;
            case '1':
                if (getchar() == ';') {
                    if (getchar() == '5') {
                        switch (getchar()) {
                        case 'C': //right by word
                            forward_word(*new_str, index);
                            break;
                        case 'D': //left by word
                            backward_word(*new_str, index);
                            break;
                        }
                    }
                }
                break;
            }
        }
    }
}

void stredit(char **str)
{
    vec2 start_pos, curr_pos, dim;
    int index = 0;
    unsigned len = 0;
    size_t n = 2 * (strlen(*str) + 1);
    char *new_str = malloc(n);
    strcpy(new_str, *str);

    start_noncannon();
    start_pos = get_cursor_pos();
    dim = get_term_size();

    curr_pos = start_pos;
    printf("%s", *str);
    set_cursor_pos(curr_pos);

    char c;
    for (;;) {
        dim = get_term_size();
        if (start_pos.x > dim.x) {
            add_wrap(&start_pos, dim, 0);
        }

        c = getchar();
        if (c == '\n')
            break;

        if (c >= ' ' && c <= '~')
            insert_char(&new_str, &n, c, &index);
        else
            do_ctrl(c, dim, start_pos, curr_pos, &new_str, &n, &index);
        vec2 end_pos;
        get_pos_from_index(new_str, start_pos, dim, strlen(new_str), &end_pos);
        for (end_pos.x = 1; end_pos.y > start_pos.y; end_pos.y--) {
            set_cursor_pos(end_pos);
            printf("\033[K");
        }
        set_cursor_pos(start_pos);
        printf("\033[K%s", new_str);
        get_pos_from_index(new_str, start_pos, dim, index, &curr_pos);
        if (end_pos.y > dim.y) {
            exit(43);
            start_pos.y -= 2;
            curr_pos.y -= 2;
        }
        set_cursor_pos(curr_pos);

    }
    *str = new_str;
    printf("\n");
    end_noncannon();
}

int main(int argc, char **argv)
{

    if (argc < 2)
        return -1;
    if (!strcmp(argv[1], "1")) {
        char *str = "Hello, world!";
        stredit(&str);
        printf("\n");
        puts(str);
    }

    if (!strcmp(argv[1], "0")) {
        start_noncannon();
        char c;
        while ((c = getchar()) != '\n') {
            printf("%o\n", c);
        }
        end_noncannon();
    }

    if (!strcmp(argv[1], "2")) {
        start_noncannon();
        vec2 dim = get_term_size();
        printf("(%u, %u)\n", dim.x, dim.y);
        end_noncannon();
    }

    if(!strcmp(argv[1], "3")) {
        start_noncannon();
        printf("\033[s\033[999;999H\033[6n\033[u");
        ungetc('f', stdin);
        scanf("\033[%*u;%*uR");
        char c;
        while (1) {
            c = getchar();
            printf("%o\n", c);
        }
        end_noncannon();
    }
}
