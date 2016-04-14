#include "memcached_time.h"
#include "stats.h"
/* time-sensitive callers can call it by hand with this, outside the normal ever-1-second timer */
void set_current_time () {
    current_time = (rel_time_t) (time(0) - stats.started);
}
