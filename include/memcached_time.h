#ifndef MEMCACHED_TIME_H
#define MEMCACHED_TIME_H
#include "stats.h"
#include <sys/time.h>
#define REALTIME_MAXDELTA 60*60*24*30
 
/* Time relative to server start. Smaller than time_t on 64-bit systems. */
typedef unsigned int rel_time_t;

/* time handling */
void set_current_time (struct stats *stats);  /* update the global variable holding
                              global 32-bit seconds-since-start time
                              (to avoid 64 bit time_t) */

rel_time_t realtime(time_t exptime, struct stats *stats);
#endif /* End definition of TIME_H */
