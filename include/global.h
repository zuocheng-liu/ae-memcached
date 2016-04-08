#ifndef GLOBAL_H
#define GLOBAL_H

#include <arpa/inet.h>
#include <sys/socket.h>

#ifndef NDEBUG
#define NDEBUG
#endif

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

/*
 * Macro functions
 */

#define LOG_DEBUG(format) LOG_DEBUG_F0(format) 
#define LOG_DEBUG_F0(format)                                 \
    do {                                                    \
        if (settings.verbose > 1) {                         \
            fprintf(stderr, format);                        \
        }                                                   \
    } while (0)
#define LOG_DEBUG_F1(format, arg0)                           \
    do {                                                    \
        if (settings.verbose > 1) {                         \
            fprintf(stderr, format, arg0);                  \
        }                                                   \
    } while (0)
#define LOG_DEBUG_F2(format, arg0, arg1)                     \
    do {                                                    \
        if (settings.verbose > 1) {                         \
            fprintf(stderr, format, arg0, arg1);            \
        }                                                   \
    } while (0)
#define LOG_DEBUG_F3(format, arg0, arg1, arg2)               \
    do {                                                    \
        if (settings.verbose > 1) {                         \
            fprintf(stderr, format, arg0, arg1, arg2);      \
        }                                                   \
    } while (0)

#define LOG_INFO(format) LOG_INFO_F0(format) 
#define LOG_INFO_F0(format)                                 \
    do {                                                    \
        if (settings.verbose > 0) {                         \
            fprintf(stderr, format);                        \
        }                                                   \
    } while (0)
#define LOG_INFO_F1(format, arg0)                           \
    do {                                                    \
        if (settings.verbose > 0) {                         \
            fprintf(stderr, format, arg0);                  \
        }                                                   \
    } while (0)
#define LOG_INFO_F2(format, arg0, arg1)                     \
    do {                                                    \
        if (settings.verbose > 0) {                         \
            fprintf(stderr, format, arg0, arg1);            \
        }                                                   \
    } while (0)
#define LOG_INFO_F3(format, arg0, arg1, arg2)               \
    do {                                                    \
        if (settings.verbose > 0) {                         \
            fprintf(stderr, format, arg0, arg1, arg2);      \
        }                                                   \
    } while (0)
#define LOG_INFO_F4(format, arg0, arg1, arg2, arg3)         \
    do {                                                    \
        if (settings.verbose > 0) {                         \
            fprintf(stderr, format, arg0 ,arg1, arg2, arg3);\
        }                                                   \
    } while (0)

#endif // end definition of GLOBAL_H
