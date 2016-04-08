#ifndef MEMCACHED_H
#define MEMCACHED_H
/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* $Id$ */


#include "config.h"
#include "global.h"
#include "ae.h"
#include "slabs.h"
#include "connection.h"

#define NREAD_ADD 1
#define NREAD_SET 2
#define NREAD_REPLACE 3

/* number of virtual buckets for a managed instance */
#define MAX_BUCKETS 32768

/* listening socket */
extern int l_socket;

/* udp socket */
extern int u_socket;

/* current time of day (updated periodically) */
extern volatile rel_time_t current_time;

/* temporary hack */
/* #define assert(x) if(!(x)) { printf("assert failure: %s\n", #x); pre_gdb(); }
   void pre_gdb (); */

/*
 * Functions
 */

/*
 * given time value that's either unix time or delta from current unix time, return
 * unix time. Use the fact that delta can't exceed one month (and real time value can't
 * be that low).
 */

rel_time_t realtime(time_t exptime);

/* event handling, network IO */
//int event_handler(int fd, short which, void *arg);
void event_handler(aeEventLoop *el, int fd, void *privdata, int mask);
void drive_machine(conn *c);
int new_socket(int isUdp);
int server_socket(int port, int isUdp);
int update_event(conn *c, int new_flags);
int try_read_command(conn *c);
int try_read_network(conn *c);
int try_read_udp(conn *c);
void complete_nread(conn *c);
void process_command(conn *c, char *command);
int transmit(conn *c);
int ensure_iov_space(conn *c);
int add_iov(conn *c, const void *buf, int len);
int add_msghdr(conn *c);
/* stats */
void stats_reset(void);
void stats_init(void);
/* defaults */
void settings_init(void);
/* time handling */
void set_current_time ();  /* update the global variable holding
                              global 32-bit seconds-since-start time
                              (to avoid 64 bit time_t) */

#endif // end definition of MEMCACHED_H
