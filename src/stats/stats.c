#include <stdio.h>
#include <time.h>

#include "stats.h"

void stat_init(stat_t* const stat) {
    stat->bytes = 0;
    stat->clock_diff = 1;
}

void stat_print(const stat_t* const stat) {
    double secs_passed = ((double)(stat->clock_diff)) / ((double)CLOCKS_PER_SEC);
    double bytes_per_sec = ((double)stat->bytes)/secs_passed;
    double kb_per_sec = bytes_per_sec / 1024.0;
    double mb_per_sec = kb_per_sec / 1024.0;
    puts("------------ Network ------------");
    printf("Average Network Speed: %08.2f KB/s (%02.2f MB/s)\n", kb_per_sec, mb_per_sec);
    puts("---------------------------------");
}