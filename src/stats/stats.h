#ifndef STATS
#define STATS

#include <time.h>

typedef struct {
    size_t bytes;
    clock_t clock_diff;
} stat_t;

void stat_init(stat_t* const stat);

void stat_print(const stat_t* const stat);
#endif