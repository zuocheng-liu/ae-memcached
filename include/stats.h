#ifndef STATS_H
#define STATS_H
#include <sys/time.h>
/* stats */
struct stats {
    unsigned int  curr_items;
    unsigned int  total_items;
    unsigned long long curr_bytes;
    unsigned int  curr_conns;
    unsigned int  total_conns;
    unsigned int  conn_structs;
    unsigned long long  get_cmds;
    unsigned long long  set_cmds;
    unsigned long long  get_hits;
    unsigned long long  get_misses;
    time_t        started;          /* when the process was started */
    unsigned long long bytes_read;
    unsigned long long bytes_written;
};

/* global variables */
extern struct stats stats;

void stats_init(void);
void stats_reset(void);
#endif /* End definition STATS_H */
