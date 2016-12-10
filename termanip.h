#pragma once

#include <stdio.h>

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

void start_noncannon(void);
void end_noncannon(void);
void set_cursor_pos(vec2 p);
int try_read_pos(char *resp, size_t *n);
vec2 get_cursor_pos(void);
vec2 get_term_size(void);
