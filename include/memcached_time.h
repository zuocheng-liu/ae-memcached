#ifndef MEMCACHED_TIME_H
#define MEMCACHED_TIME_H

/* Time relative to server start. Smaller than time_t on 64-bit systems. */
typedef unsigned int rel_time_t;

/* global variable */
extern volatile rel_time_t current_time;

/* time handling */
void set_current_time ();  /* update the global variable holding
                              global 32-bit seconds-since-start time
                              (to avoid 64 bit time_t) */

#endif /* End definition of TIME_H */
