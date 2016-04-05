#ifndef GLOBAL_H
#define GLOBAL_H

#include <arpa/inet.h>
#include <sys/socket.h>

/* Time relative to server start. Smaller than time_t on 64-bit systems. */
typedef unsigned int rel_time_t;

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

struct settings {
    size_t maxbytes;
    int maxconns;
    int port;
    int udpport;
    struct in_addr interface;
    int verbose;
    rel_time_t oldest_live; /* ignore existing items older than this */
    int managed;          /* if 1, a tracker manages virtual buckets */
    int evict_to_free;
    char *socketpath;   /* path to unix socket if using local socket */
    double factor;          /* chunk size growth factor */
    int chunk_size;
};

extern struct stats stats;
extern struct settings settings;


#endif // end definition of GLOBAL_H
