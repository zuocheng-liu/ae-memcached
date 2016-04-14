#ifndef SETTINGS_H
#define SETTINGS_H
#include <stdlib.h>
#include <arpa/inet.h>
#include "memcached_time.h"

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
    int lock_memory;
    int daemonize;
    int maxcore;
    char *username;
    char *pid_file;
};


u_int32_t process_arguments(struct settings *settings, int argc, char **argv);
void usage(void); 
void usage_license(void); 
#endif /* End definition of SETTINGS_H */
