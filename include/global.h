#ifndef GLOBAL_H
#define GLOBAL_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "ae.h"
#include "command.h"
#include "settings.h"
#include "stats.h"
#include "logger.h"

/*
 * Global variables
 */
extern struct settings settings;
extern struct stats stats;
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

#endif /* end definition of GLOBAL_H */
