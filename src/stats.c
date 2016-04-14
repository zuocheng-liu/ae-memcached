#include "stats.h"
#include "settings.h"

void stats_init(struct stats *stats) {
    stats->curr_items = 0;
    stats->total_items = 0;
    stats->curr_conns = 0;
    stats->total_conns = 0;
    stats->conn_structs = 0;
    stats->get_cmds = 0;
    stats->set_cmds = 0;
    stats->get_hits = 0;
    stats->get_misses = 0;
    stats->curr_bytes = 0;
    stats->bytes_read = 0;
    stats->bytes_written = 0;

    /* make the time we started always be 1 second before we really
       did, so time(0) - time.started is never zero.  if so, things
       like 'settings.oldest_live' which act as booleans as well as
       values are now false in boolean context... */
    stats->started = time(0) - 1;
}

void stats_reset(struct stats *stats) {
    stats->total_items = 0;
    stats->total_conns = 0;
    stats->get_cmds = 0;
    stats->set_cmds = 0;
    stats->get_hits = 0;
    stats->get_misses = 0;
    stats->bytes_read = 0;
    stats->bytes_written = 0;
}

