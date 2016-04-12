#include "global.h"

struct aeEventLoop *g_el;
command_service_ptr g_cmd_srv;

struct stats stats;
struct settings settings;

volatile rel_time_t current_time;

/* time-sensitive callers can call it by hand with this, outside the normal ever-1-second timer */
void set_current_time () {
    current_time = (rel_time_t) (time(0) - stats.started);
}


void settings_init(void) {
    settings.port = 11211;
    settings.udpport = 0;
    settings.interface.s_addr = htonl(INADDR_ANY);
    settings.maxbytes = 64*1024*1024; /* default is 64MB */
    settings.maxconns = 1024;         /* to limit connections-related memory to about 5MB */
    settings.verbose = 0;
    settings.oldest_live = 0;
    settings.evict_to_free = 1;       /* push old items out of cache when memory runs out */
    settings.socketpath = NULL;       /* by default, not using a unix socket */
    settings.managed = 0;
    settings.factor = 1.25;
    settings.chunk_size = 48;         /* space for a modest key and value */
}
