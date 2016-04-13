#ifndef GLOBAL_H
#define GLOBAL_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "ae.h"
#include "command.h"

#define VERSION "1.2.0"
#define PACKAGE "ae-memached"

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

/*
 * Global variables
 */
extern struct stats stats;
extern struct settings settings;
extern struct aeEventLoop *g_el;
extern command_service_ptr g_cmd_srv;

/*
 * We keep the current time of day in a global variable that's updated by a
 * timer event. This saves us a bunch of time() system calls (we really only
 * need to get the time once a second, whereas there can be tens of thousands
 * of requests a second) and allows us to use server-start-relative timestamps
 * rather than absolute UNIX timestamps, a space savings on systems where
 * sizeof(time_t) > sizeof(unsigned int).
 */
extern volatile rel_time_t current_time;

extern struct aeEventLoop *g_el;

/*
 * Macro functions
 */

/* Logger functions */
#define LOG_PRINT(format) LOG_PRINT_F0(format)
#define LOG_PRINT_F0(format)                                \
    fprintf(stderr, format)
#define LOG_PRINT_F1(format,arg0)                           \
    fprintf(stderr, format, arg0)
#define LOG_PRINT_F2(format,arg0, arg1)                     \
    fprintf(stderr, format, arg0, arg1)
#define LOG_PRINT_F3(format,arg0, arg1, arg2)               \
    fprintf(stderr, format, arg0, arg1, arg2)
#define LOG_PRINT_F4(format,arg0, arg1, arg2, arg3)         \
    fprintf(stderr, format, arg0, arg1, arg2, arg3)

#define LOG_DEBUG(format) LOG_DEBUG_F0(format) 
#define LOG_DEBUG_F0(format)                                \
    do {                                                    \
        if (settings.verbose > 1) {                         \
            LOG_PRINT_F0(format);                           \
        }                                                   \
    } while (0)
#define LOG_DEBUG_F1(format, arg0)                          \
    do {                                                    \
        if (settings.verbose > 1) {                         \
            LOG_PRINT_F1(format, arg0);                     \
        }                                                   \
    } while (0)
#define LOG_DEBUG_F2(format, arg0, arg1)                    \
    do {                                                    \
        if (settings.verbose > 1) {                         \
            LOG_PRINT_F2(format, arg0, arg1);               \
        }                                                   \
    } while (0)
#define LOG_DEBUG_F3(format, arg0, arg1, arg2)              \
    do {                                                    \
        if (settings.verbose > 1) {                         \
            LOG_PRINT_F3(format, arg0, arg1, arg2);         \
        }                                                   \
    } while (0)
#define LOG_DEBUG_F4(format, arg0, arg1, arg2, arg3)        \
    do {                                                    \
        if (settings.verbose > 1) {                         \
            LOG_PRINT_F4(format, arg0, arg1, arg2, arg3);   \
        }                                                   \
    } while (0)


#define LOG_INFO(format) LOG_INFO_F0(format) 
#define LOG_INFO_F0(format)                                 \
    do {                                                    \
        if (settings.verbose > 0) {                         \
            LOG_PRINT_F0(format);                           \
        }                                                   \
    } while (0)
#define LOG_INFO_F1(format, arg0)                           \
    do {                                                    \
        if (settings.verbose > 0) {                         \
            LOG_PRINT_F1(format, arg0);                     \
        }                                                   \
    } while (0)
#define LOG_INFO_F2(format, arg0, arg1)                     \
    do {                                                    \
        if (settings.verbose > 0) {                         \
            LOG_PRINT_F2(format, arg0, arg1);               \
        }                                                   \
    } while (0)
#define LOG_INFO_F3(format, arg0, arg1, arg2)               \
    do {                                                    \
        if (settings.verbose > 0) {                         \
            LOG_PRINT_F3(format, arg0, arg1, arg2);         \
        }                                                   \
    } while (0)
#define LOG_INFO_F4(format, arg0, arg1, arg2, arg3)         \
    do {                                                    \
        if (settings.verbose > 0) {                         \
            LOG_PRINT_F4(format, arg0 ,arg1, arg2, arg3);   \
        }                                                   \
    } while (0)

#define LOG_WARN(format) LOG_WARN_F0(format) 
#define LOG_WARN_F0(format)                                 \
    do {                                                    \
        LOG_PRINT_F0(format);                               \
    } while (0)
#define LOG_WARN_F1(format, arg0)                           \
    do {                                                    \
        LOG_PRINT_F1(format, arg0);                         \
    } while (0)

#define LOG_ERROR(format) LOG_ERROR_F0(format) 
#define LOG_ERROR_F0(format)                                \
    do {                                                    \
        LOG_PRINT_F0(format);                               \
    } while (0)
#define LOG_ERROR_F1(format, arg0)                          \
    do {                                                    \
        LOG_PRINT_F1(format, arg0);                         \
    } while (0)


#define LOG_FATAL(format) LOG_FATAL_F0(format) 
#define LOG_FATAL_F0(format)                                \
    do {                                                    \
        LOG_PRINT_F0(format);                                  \
        exit(-1);                                           \
    } while (0)
#define LOG_FATAL_F1(format, arg0)                          \
    do {                                                    \
        LOG_PRINT_F1(format, arg0);                            \
        exit(-1);                                           \
    } while (0)


/* defaults */
void settings_init(void);
/* time handling */
void set_current_time ();  /* update the global variable holding
                              global 32-bit seconds-since-start time
                              (to avoid 64 bit time_t) */


#endif /* end definition of GLOBAL_H */
