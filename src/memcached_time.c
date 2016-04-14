#include "memcached_time.h"
#include "stats.h"
#include "global.h"
#include <time.h>
#include <stdio.h>
/* time-sensitive callers can call it by hand with this, outside the normal ever-1-second timer */
void set_current_time (struct stats *stats) {
    current_time = (rel_time_t) (time(0) - stats->started);
}

rel_time_t realtime(time_t exptime, struct stats *stats) {
    /* no. of seconds in 30 days - largest possible delta exptime */

    if (exptime == 0) return 0; /* 0 means never expire */

    if (exptime > REALTIME_MAXDELTA)
        return (rel_time_t) (exptime - stats->started);
    else {
        return (rel_time_t) (exptime + current_time);
    }
}
