/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *  memcached - memory caching daemon
 *
 *       http://www.danga.com/memcached/
 *
 *  Copyright 2003 Danga Interactive, Inc.  All rights reserved.
 *
 *  Use and distribution licensed under the BSD license.  See
 *  the LICENSE file for full text.
 *
 *  Authors:
 *      Anatoly Vorobey <mellon@pobox.com>
 *      Brad Fitzpatrick <brad@danga.com>
 *
 *  $Id$
 */

/* some POSIX systems need the following definition
 * to get mlockall flags out of sys/mman.h.  */
#ifndef _P1003_1B_VISIBLE
#define _P1003_1B_VISIBLE
#endif


#include <sys/signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/resource.h>
#include <sys/uio.h>

#include <pwd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  

#include "ae.h"
#include "assoc.h"
#include "connection.h"
#include "memcached.h"
#include "global.h"

static item **todelete = 0;
static int delcurr;
static int deltotal;


#define TRANSMIT_COMPLETE   0
#define TRANSMIT_INCOMPLETE 1
#define TRANSMIT_SOFT_ERROR 2
#define TRANSMIT_HARD_ERROR 3

int *buckets = 0; /* bucket->generation array for a managed instance */

/* returns true if a deleted item's delete-locked-time is over, and it
   should be removed from the namespace */
int item_delete_lock_over (item *it) {
    assert(it->it_flags & ITEM_DELETED);
    return (current_time >= it->exptime);
}

/* wrapper around assoc_find which does the lazy expiration/deletion logic */
item *get_item_notedeleted(char *key, int *delete_locked) {
    item *it = assoc_find(key);
    if (delete_locked) *delete_locked = 0;
    if (it && (it->it_flags & ITEM_DELETED)) {
        /* it's flagged as delete-locked.  let's see if that condition
           is past due, and the 5-second delete_timer just hasn't
           gotten to it yet... */
        if (! item_delete_lock_over(it)) {
            if (delete_locked) *delete_locked = 1;
            it = 0;
        }
    }
    if (it && settings.oldest_live && settings.oldest_live <= current_time &&
        it->time <= settings.oldest_live) {
        item_unlink(it);
        it = 0;
    }
    if (it && it->exptime && it->exptime <= current_time) {
        item_unlink(it);
        it = 0;
    }
    return it;
}

item *get_item(char *key) {
    return get_item_notedeleted(key, 0);
}

/*
 * Ensures that there is room for another struct iovec in a connection's
 * iov list.
 *
 * Returns 0 on success, -1 on out-of-memory.
 */
int ensure_iov_space(conn *c) {
    if (c->iovused >= c->iovsize) {
        int i, iovnum;
        struct iovec *new_iov = (struct iovec *) realloc(c->iov,
                                (c->iovsize * 2) * sizeof(struct iovec));
        if (! new_iov)
            return -1;
        c->iov = new_iov;
        c->iovsize *= 2;

        /* Point all the msghdr structures at the new list. */
        for (i = 0, iovnum = 0; i < c->msgused; i++) {
            c->msglist[i].msg_iov = &c->iov[iovnum];
            iovnum += c->msglist[i].msg_iovlen;
        }
    }

    return 0;
}


/*
 * Constructs a set of UDP headers and attaches them to the outgoing messages.
 */
int build_udp_headers(conn *c) {
    int i;
    unsigned char *hdr;

    if (c->msgused > c->hdrsize) {
        void *new_hdrbuf;
        if (c->hdrbuf)
            new_hdrbuf = realloc(c->hdrbuf, c->msgused * 2 * UDP_HEADER_SIZE);
        else
            new_hdrbuf = malloc(c->msgused * 2 * UDP_HEADER_SIZE);
        if (! new_hdrbuf)
            return -1;
        c->hdrbuf = (unsigned char *) new_hdrbuf;
        c->hdrsize = c->msgused * 2;
    }

    hdr = c->hdrbuf;
    for (i = 0; i < c->msgused; i++) {
        c->msglist[i].msg_iov[0].iov_base = hdr;
        c->msglist[i].msg_iov[0].iov_len = UDP_HEADER_SIZE;
        *hdr++ = c->request_id / 256;
        *hdr++ = c->request_id % 256;
        *hdr++ = i / 256;
        *hdr++ = i % 256;
        *hdr++ = c->msgused / 256;
        *hdr++ = c->msgused % 256;
        *hdr++ = 0;
        *hdr++ = 0;
        assert((void*) hdr == (void*) c->msglist[i].msg_iov[0].iov_base + UDP_HEADER_SIZE);
    }

    return 0;
}



/*
 * we get here after reading the value in set/add/replace commands. The command
 * has been stored in c->item_comm, and the item is ready in c->item.
 */

void complete_nread(conn *c) {
    item *it = c->item;
    int comm = c->item_comm;
    item *old_it;
    int delete_locked = 0;
    char *key = ITEM_key(it);

    stats.set_cmds++;

    if (strncmp(ITEM_data(it) + it->nbytes - 2, "\r\n", 2) != 0) {
        out_string(c, "CLIENT_ERROR bad data chunk");
        goto err;
    }

    old_it = get_item_notedeleted(key, &delete_locked);

    if (old_it && comm == NREAD_ADD) {
        item_update(old_it);  /* touches item, promotes to head of LRU */
        out_string(c, "NOT_STORED");
        goto err;
    }

    if (!old_it && comm == NREAD_REPLACE) {
        out_string(c, "NOT_STORED");
        goto err;
    }

    if (delete_locked) {
        if (comm == NREAD_REPLACE || comm == NREAD_ADD) {
            out_string(c, "NOT_STORED");
            goto err;
        }

        /* but "set" commands can override the delete lock
         window... in which case we have to find the old hidden item
         that's in the namespace/LRU but wasn't returned by
         get_item.... because we need to replace it (below) */
        old_it = assoc_find(key);
    }

    if (old_it)
        item_replace(old_it, it);
    else
        item_link(it);

    c->item = 0;
    out_string(c, "STORED");
    return;

err:
     item_free(it);
     c->item = 0;
     return;
}

u_int32_t command_get(char *command, conn *c, int binary) {
    char key[251];
    int next;
    item *it;
    char *start = command + 4;
    int i = 0;
    if (settings.managed) {
        int bucket = c->bucket;
        if (bucket == -1) {
            out_string(c, "CLIENT_ERROR no BG data in managed mode");
            return COMMAND_OK;
        }
        c->bucket = -1;
        if (buckets[bucket] != c->gen) {
            out_string(c, "ERROR_NOT_OWNER");
            return COMMAND_OK;
        }
    }

    while(sscanf(start, " %250s%n", key, &next) >= 1) {
        start+=next;
        stats.get_cmds++;
        it = get_item(key);
        if (it) {
            if (i >= c->isize) {
                item **new_list = realloc(c->ilist, sizeof(item *)*c->isize*2);
                if (new_list) {
                    c->isize *= 2;
                    c->ilist = new_list;
                } else break;
            }

            /*
             * Construct the response. Each hit adds three elements to the
             * outgoing data list:
             *   "VALUE "
             *   key
             *   " " + flags + " " + data length + "\r\n" + data (with \r\n)
             */
            /* TODO: can we avoid the strlen() func call and cache that in wasted byte in item struct? */
            if (add_iov(c, "VALUE ", 6) ||
                    add_iov(c, ITEM_key(it), strlen(ITEM_key(it))) ||
                    add_iov(c, ITEM_suffix(it), it->nsuffix + it->nbytes))
            {
                break;
            }

            LOG_DEBUG_F2(">%d sending key %s\n", c->sfd, ITEM_key(it));

            stats.get_hits++;
            it->refcount++;
            item_update(it);
            *(c->ilist + i) = it;
            i++;
        } else stats.get_misses++;
    }

    c->icurr = c->ilist;
    c->ileft = i;

    LOG_DEBUG_F1(">%d END\n", c->sfd);
    add_iov(c, "END\r\n", 5);

    if (c->udp && build_udp_headers(c)) {
        out_string(c, "SERVER_ERROR out of memory");
    }
    else {
        conn_set_state(c, conn_mwrite);
        c->msgcurr = 0;
    }
    return COMMAND_OK;
}

u_int32_t bget_handler(char *command, int argc, char ** argv) {
    ((conn *)argv)->binary = 1;
    return command_get(command, (conn *)argv, 1);
}

u_int32_t get_handler(char *command, int argc, char ** argv) {
    return command_get(command, (conn *)argv, 0);
}

/*
 * for commands set/add/replace, we build an item and read the data
 * directly into it, then continue in nread_complete().
 */
u_int32_t command_replace(char *command, conn *c, int com_type) {
    int comm = com_type;
    char key[251];
    int flags;
    time_t expire;
    int len, res;
    item *it;

    res = sscanf(command, "%*s %250s %u %ld %d\n", key, &flags, &expire, &len);
    if (res!=4 || strlen(key)==0 ) {
        out_string(c, "CLIENT_ERROR bad command line format");
        return COMMAND_OK;
    }

    if (settings.managed) {
        int bucket = c->bucket;
        if (bucket == -1) {
            out_string(c, "CLIENT_ERROR no BG data in managed mode");
            return COMMAND_OK;
        }
        c->bucket = -1;
        if (buckets[bucket] != c->gen) {
            out_string(c, "ERROR_NOT_OWNER");
            return COMMAND_OK;
        }
    }

    expire = realtime(expire, &stats);
    it = item_alloc(key, flags, expire, len+2);

    if (it == 0) {
        if (! item_size_ok(key, flags, len + 2))
            out_string(c, "SERVER_ERROR object too large for cache");
        else
            out_string(c, "SERVER_ERROR out of memory");
        /* swallow the data line */
        c->write_and_go = conn_swallow;
        c->sbytes = len+2;
        return COMMAND_OK;
    }

    c->item_comm = comm;
    c->item = it;
    c->ritem = ITEM_data(it);
    c->rlbytes = it->nbytes;
    conn_set_state(c, conn_nread);
    return COMMAND_OK;
}

u_int32_t replace_handler(char *command, int argc, char ** argv) {
    return command_replace(command, (conn *)argv, NREAD_REPLACE);
}

u_int32_t set_handler(char *command, int argc, char ** argv) {
    return command_replace(command, (conn *)argv, NREAD_SET);
}

u_int32_t add_handler(char *command, int argc, char ** argv) {
    return command_replace(command, (conn *)argv, NREAD_ADD);
}

u_int32_t command_vary_delta(char *command, conn *c, int incr) {
    char temp[32];
    unsigned int value;
    item *it;
    unsigned int delta;
    char key[251];
    int res;
    char *ptr;
    res = sscanf(command, "%*s %250s %u\n", key, &delta);
    if (res!=2 || strlen(key)==0 ) {
        out_string(c, "CLIENT_ERROR bad command line format");
        return COMMAND_OK;
    }
    if (settings.managed) {
        int bucket = c->bucket;
        if (bucket == -1) {
            out_string(c, "CLIENT_ERROR no BG data in managed mode");
            return COMMAND_OK;
        }
        c->bucket = -1;
        if (buckets[bucket] != c->gen) {
            out_string(c, "ERROR_NOT_OWNER");
            return COMMAND_OK;
        }
    }
    it = get_item(key);
    if (!it) {
        out_string(c, "NOT_FOUND");
        return COMMAND_OK;
    }
    ptr = ITEM_data(it);
    while (*ptr && (*ptr<'0' && *ptr>'9')) ptr++;    /* BUG: can't be true */
    value = atoi(ptr);
    if (incr)
        value+=delta;
    else {
        if (delta >= value) value = 0;
        else value-=delta;
    }
    sprintf(temp, "%u", value);
    res = strlen(temp);
    if (res + 2 > it->nbytes) { /* need to realloc */
        item *new_it;
        new_it = item_alloc(ITEM_key(it), atoi(ITEM_suffix(it) + 1), it->exptime, res + 2 );
        if (new_it == 0) {
            out_string(c, "SERVER_ERROR out of memory");
            return COMMAND_OK;
        }
        memcpy(ITEM_data(new_it), temp, res);
        memcpy(ITEM_data(new_it) + res, "\r\n", 2);
        item_replace(it, new_it);
    } else { /* replace in-place */
        memcpy(ITEM_data(it), temp, res);
        memset(ITEM_data(it) + res, ' ', it->nbytes-res-2);
    }
    out_string(c, temp);
    return COMMAND_OK;
}

u_int32_t incr_handler(char *command, int argc, char ** argv) {
    return command_vary_delta(command, (conn *)argv, 1);
}

u_int32_t decr_handler(char *command, int argc, char ** argv) {
    return command_vary_delta(command, (conn *)argv, 0);
}

u_int32_t delete_handler(char *command, int argc, char ** argv) {
    char key[251];
    item *it;
    int res;
    time_t exptime = 0;
    conn *c = (conn*)argv;

    if (settings.managed) {
        int bucket = c->bucket;
        if (bucket == -1) {
            out_string(c, "CLIENT_ERROR no BG data in managed mode");
            return COMMAND_OK;
        }
        c->bucket = -1;
        if (buckets[bucket] != c->gen) {
            out_string(c, "ERROR_NOT_OWNER");
            return COMMAND_OK;
        }
    }
    res = sscanf(command, "%*s %250s %ld", key, &exptime);
    if (res < 0) {
        out_string(c, "ERROR");
        return COMMAND_OK;
    }
    it = get_item(key);
    if (!it) {
        out_string(c, "NOT_FOUND");
        return COMMAND_OK;
    }
    if (exptime == 0) {
        item_unlink(it);
        out_string(c, "DELETED");
        return COMMAND_OK;
    }
    if (delcurr >= deltotal) {
        item **new_delete = realloc(todelete, sizeof(item *) * deltotal * 2);
        if (new_delete) {
            todelete = new_delete;
            deltotal *= 2;
        } else {
            /*
             * can't delete it immediately, user wants a delay,
             * but we ran out of memory for the delete queue
             */
            out_string(c, "SERVER_ERROR out of memory");
            return COMMAND_OK;
        }
    }

    it->refcount++;
    /* use its expiration time as its deletion time now */
    it->exptime = realtime(exptime, &stats);
    it->it_flags |= ITEM_DELETED;
    todelete[delcurr++] = it;
    out_string(c, "DELETED");
    return COMMAND_OK;
}

u_int32_t own_handler(char *command, int argc, char ** argv) {
    int bucket, gen;
    char *start = command+4;
    conn *c = (conn*)argv;
    if (!settings.managed) {
        out_string(c, "CLIENT_ERROR not a managed instance");
        return COMMAND_OK;
    }
    if (sscanf(start, "%u:%u\r\n", &bucket,&gen) == 2) {
        if ((bucket < 0) || (bucket >= MAX_BUCKETS)) {
            out_string(c, "CLIENT_ERROR bucket number out of range");
            return COMMAND_OK;
        }
        buckets[bucket] = gen;
        out_string(c, "OWNED");
        return COMMAND_OK;
    } else {
        out_string(c, "CLIENT_ERROR bad format");
        return COMMAND_OK;
    }
}

u_int32_t disown_handler(char *command, int argc, char ** argv) {
    int bucket;
    char *start = command+7;
    conn *c = (conn*)argv;
    if (!settings.managed) {
        out_string(c, "CLIENT_ERROR not a managed instance");
        return COMMAND_OK;
    }
    if (sscanf(start, "%u\r\n", &bucket) == 1) {
        if ((bucket < 0) || (bucket >= MAX_BUCKETS)) {
            out_string(c, "CLIENT_ERROR bucket number out of range");
            return COMMAND_OK;
        }
        buckets[bucket] = 0;
        out_string(c, "DISOWNED");
        return COMMAND_OK;
    } else {
        out_string(c, "CLIENT_ERROR bad format");
        return COMMAND_OK;
    }
}

u_int32_t stats_handler(char *cmd_s, int argc, char ** argv) {
    conn *c = (conn*)argv;
    rel_time_t now = current_time;
    char temp[1024];
    pid_t pid = getpid();
    char *pos = temp;
    struct rusage usage;

    getrusage(RUSAGE_SELF, &usage);

    pos += sprintf(pos, "STAT pid %u\r\n", pid);
    pos += sprintf(pos, "STAT uptime %u\r\n", now);
    pos += sprintf(pos, "STAT time %ld\r\n", now + stats.started);
    pos += sprintf(pos, "STAT version " VERSION "\r\n");
    pos += sprintf(pos, "STAT pointer_size %ld\r\n", 8 * sizeof(void*));
    pos += sprintf(pos, "STAT rusage_user %ld.%06ld\r\n", usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
    pos += sprintf(pos, "STAT rusage_system %ld.%06ld\r\n", usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
    pos += sprintf(pos, "STAT curr_items %u\r\n", stats.curr_items);
    pos += sprintf(pos, "STAT total_items %u\r\n", stats.total_items);
    pos += sprintf(pos, "STAT bytes %llu\r\n", stats.curr_bytes);
    pos += sprintf(pos, "STAT curr_connections %u\r\n", stats.curr_conns - 1); /* ignore listening conn */
    pos += sprintf(pos, "STAT total_connections %u\r\n", stats.total_conns);
    pos += sprintf(pos, "STAT connection_structures %u\r\n", stats.conn_structs);
    pos += sprintf(pos, "STAT cmd_get %llu\r\n", stats.get_cmds);
    pos += sprintf(pos, "STAT cmd_set %llu\r\n", stats.set_cmds);
    pos += sprintf(pos, "STAT get_hits %llu\r\n", stats.get_hits);
    pos += sprintf(pos, "STAT get_misses %llu\r\n", stats.get_misses);
    pos += sprintf(pos, "STAT bytes_read %llu\r\n", stats.bytes_read);
    pos += sprintf(pos, "STAT bytes_written %llu\r\n", stats.bytes_written);
    pos += sprintf(pos, "STAT limit_maxbytes %llu\r\n", (unsigned long long) settings.maxbytes);
    pos += sprintf(pos, "END");
    out_string(c, temp);
    return COMMAND_OK;
}

u_int32_t stats_reset_handler(char *cmd_s, int argc, char ** argv) {
    conn *c = (conn*)argv;
    stats_reset(&stats);
    out_string(c, "RESET");
    return COMMAND_OK;
}

u_int32_t stats_malloc_handler(char *cmd_s, int argc, char ** argv) {
#ifdef HAVE_MALLOC_H
#ifdef HAVE_STRUCT_MALLINFO
    char temp[512];
    struct mallinfo info;
    char *pos = temp;
    conn *c = (conn*)argv;
    rel_time_t now = current_time;
    info = mallinfo();
    pos += sprintf(pos, "STAT arena_size %d\r\n", info.arena);
    pos += sprintf(pos, "STAT free_chunks %d\r\n", info.ordblks);
    pos += sprintf(pos, "STAT fastbin_blocks %d\r\n", info.smblks);
    pos += sprintf(pos, "STAT mmapped_regions %d\r\n", info.hblks);
    pos += sprintf(pos, "STAT mmapped_space %d\r\n", info.hblkhd);
    pos += sprintf(pos, "STAT max_total_alloc %d\r\n", info.usmblks);
    pos += sprintf(pos, "STAT fastbin_space %d\r\n", info.fsmblks);
    pos += sprintf(pos, "STAT total_alloc %d\r\n", info.uordblks);
    pos += sprintf(pos, "STAT total_free %d\r\n", info.fordblks);
    pos += sprintf(pos, "STAT releasable_space %d\r\nEND", info.keepcost);
    out_string(c, temp);
#endif /* HAVE_STRUCT_MALLINFO */
#endif /* HAVE_MALLOC_H */
    return COMMAND_OK;
}

u_int32_t stats_maps_handler(char *cmd_s, int argc, char ** argv) {
    char *wbuf;
    int wsize = 8192; /* should be enough */
    int fd;
    int res;
    conn *c = (conn*)argv;

    wbuf = (char *)malloc(wsize);
    if (wbuf == 0) {
        out_string(c, "SERVER_ERROR out of memory");
        return COMMAND_OK;
    }

    fd = open("/proc/self/maps", O_RDONLY);
    if (fd == -1) {
        out_string(c, "SERVER_ERROR cannot open the maps file");
        free(wbuf);
        return COMMAND_OK;
    }

    res = read(fd, wbuf, wsize - 6);  /* 6 = END\r\n\0 */
    if (res == wsize - 6) {
        out_string(c, "SERVER_ERROR buffer overflow");
        free(wbuf); close(fd);
        return COMMAND_OK;
    }
    if (res == 0 || res == -1) {
        out_string(c, "SERVER_ERROR can't read the maps file");
        free(wbuf); close(fd);
        return COMMAND_OK;
    }
    strcpy(wbuf + res, "END\r\n");
    c->write_and_free=wbuf;
    c->wcurr=wbuf;
    c->wbytes = res + 6;
    conn_set_state(c, conn_write);
    c->write_and_go = conn_read;
    close(fd);

    return COMMAND_OK;
}


u_int32_t stats_cachedump_handler(char *command, int argc, char ** argv) {
    char *buf;
    unsigned int bytes, id, limit = 0;
    conn *c = (conn*)argv;
    char *start = command + 15;
    if (sscanf(start, "%u %u\r\n", &id, &limit) < 1) {
        out_string(c, "CLIENT_ERROR bad command line");
        return COMMAND_OK;
    }

    buf = item_cachedump(id, limit, &bytes);
    if (buf == 0) {
        out_string(c, "SERVER_ERROR out of memory");
        return COMMAND_OK;
    }

    c->write_and_free = buf;
    c->wcurr = buf;
    c->wbytes = bytes;
    conn_set_state(c, conn_write);
    c->write_and_go = conn_read;

    return COMMAND_OK;
}

u_int32_t stats_slabs_handler(char *cmd_s, int argc, char ** argv) {
    int bytes = 0;
    char *buf = slabs_stats(&bytes);
    conn *c = (conn*)argv;
    if (!buf) {
        out_string(c, "SERVER_ERROR out of memory");
        return COMMAND_OK;
    }
    c->write_and_free = buf;
    c->wcurr = buf;
    c->wbytes = bytes;
    conn_set_state(c, conn_write);
    c->write_and_go = conn_read;

    return COMMAND_OK;
}

u_int32_t stats_items_handler(char *cmd_s, int argc, char ** argv) {
    conn *c = (conn*)argv;
    char buffer[4096];
    item_stats(buffer, 4096);
    out_string(c, buffer);
    return COMMAND_OK;
}

u_int32_t stats_sizes_handler(char *cmd_s, int argc, char ** argv) {
    int bytes = 0;
    char *buf = item_stats_sizes(&bytes);
    conn *c = (conn*)argv;
    if (! buf) {
        out_string(c, "SERVER_ERROR out of memory");
        return COMMAND_OK;
    }

    c->write_and_free = buf;
    c->wcurr = buf;
    c->wbytes = bytes;
    conn_set_state(c, conn_write);
    c->write_and_go = conn_read;

    return COMMAND_OK;
}

u_int32_t version_handler(char *cmd_s, int argc, char ** argv) {
    conn *c = (conn*)argv;
    out_string(c, "VERSION " VERSION);
    return COMMAND_OK;
}

u_int32_t quit_handler(char *cmd_s, int argc, char ** argv) {
    conn *c = (conn*)argv;
    conn_set_state(c, conn_closing);
    return COMMAND_OK;
}

u_int32_t slabs_reassign_handler(char *command, int argc, char ** argv) {
    conn *c = (conn*)argv;
#ifdef ALLOW_SLABS_REASSIGN
    int src, dst;
    char *start = command + 15;
    if (sscanf(start, "%u %u\r\n", &src, &dst) == 2) {
        int rv = slabs_reassign(src, dst);
        if (rv == 1) {
            out_string(c, "DONE");
            return;
        }
        if (rv == 0) {
            out_string(c, "CANT");
            return;
        }
        if (rv == -1) {
            out_string(c, "BUSY");
            return;
        }
    }
    out_string(c, "CLIENT_ERROR bogus command");
#else
    out_string(c, "CLIENT_ERROR Slab reassignment not supported");
#endif
    return COMMAND_OK;
}

u_int32_t flush_all_handler(char *command, int argc, char ** argv) {
    time_t exptime = 0;
    int res;
    conn *c = (conn*)argv;
    set_current_time(&stats);

    if (strcmp(command, "flush_all") == 0) {
        settings.oldest_live = current_time;
        out_string(c, "OK");
        return COMMAND_OK;
    }

    res = sscanf(command, "%*s %ld", &exptime);
    if (res != 1) {
        out_string(c, "ERROR");
        return COMMAND_OK;
    }

    settings.oldest_live = realtime(exptime, &stats);
    out_string(c, "OK");
    return COMMAND_OK;
}

u_int32_t bg_handler(char *command, int argc, char ** argv) {
    int bucket, gen;
    char *start = command + 3;
    conn *c = (conn*)argv;
    if (!settings.managed) {
        out_string(c, "CLIENT_ERROR not a managed instance");
        return COMMAND_OK;
    }
    if (sscanf(start, "%u:%u\r\n", &bucket,&gen) == 2) {
        /* we never write anything back, even if input's wrong */
        if ((bucket < 0) || (bucket >= MAX_BUCKETS) || (gen<=0)) {
            /* do nothing, bad input */
        } else {
            c->bucket = bucket;
            c->gen = gen;
        }
        conn_set_state(c, conn_read);
        return COMMAND_OK;
    } else {
        out_string(c, "CLIENT_ERROR bad format");
        return COMMAND_OK;
    }
}


void process_command(conn *c, char *command) {

    LOG_DEBUG_F2("<%d %s\n", c->sfd, command);
    c->msgcurr = 0;
    c->msgused = 0;
    c->iovused = 0;
    if (add_msghdr(c)) {
        out_string(c, "SERVER_ERROR out of memory");
        return;
    }

    if(command_service_run(g_cmd_srv, command, 1, (char **)c) < 0) {
        out_string(c, "ERROR");
    }
    return;
}

/*
 * if we have a complete line in the buffer, process it.
 */
int try_read_command(conn *c) {
    char *el, *cont;

    if (!c->rbytes)
        return 0;
    el = memchr(c->rcurr, '\n', c->rbytes);
    if (!el)
        return 0;
    cont = el + 1;
    if (el - c->rcurr > 1 && *(el - 1) == '\r') {
        el--;
    }
    *el = '\0';

    process_command(c, c->rcurr);

    c->rbytes -= (cont - c->rcurr);
    c->rcurr = cont;

    return 1;
}

/*
 * read a UDP request.
 * return 0 if there's nothing to read.
 */
int try_read_udp(conn *c) {
    int res;

    c->request_addr_size = sizeof(c->request_addr);
    res = recvfrom(c->sfd, c->rbuf, c->rsize,
                   0, &c->request_addr, &c->request_addr_size);
    if (res > 8) {
        unsigned char *buf = (unsigned char *)c->rbuf;
        stats.bytes_read += res;

        /* Beginning of UDP packet is the request ID; save it. */
        c->request_id = buf[0] * 256 + buf[1];

        /* If this is a multi-packet request, drop it. */
        if (buf[4] != 0 || buf[5] != 1) {
            out_string(c, "SERVER_ERROR multi-packet request not supported");
            return 0;
        }

        /* Don't care about any of the rest of the header. */
        res -= 8;
        memmove(c->rbuf, c->rbuf + 8, res);

        c->rbytes += res;
        c->rcurr = c->rbuf;
        return 1;
    }
    return 0;
}

/*
 * read from network as much as we can, handle buffer overflow and connection
 * close.
 * before reading, move the remaining incomplete fragment of a command
 * (if any) to the beginning of the buffer.
 * return 0 if there's nothing to read on the first read.
 */
int try_read_network(conn *c) {
    int gotdata = 0;
    int res;

    if (c->rcurr != c->rbuf) {
        if (c->rbytes != 0) /* otherwise there's nothing to copy */
            memmove(c->rbuf, c->rcurr, c->rbytes);
        c->rcurr = c->rbuf;
    }

    while (1) {
        if (c->rbytes >= c->rsize) {
            char *new_rbuf = realloc(c->rbuf, c->rsize*2);
            if (!new_rbuf) {
                LOG_INFO("Couldn't realloc input buffer\n");
                c->rbytes = 0; /* ignore what we read */
                out_string(c, "SERVER_ERROR out of memory");
                c->write_and_go = conn_closing;
                return 1;
            }
            c->rcurr  = c->rbuf = new_rbuf;
            c->rsize *= 2;
        }

        /* unix socket mode doesn't need this, so zeroed out.  but why
         * is this done for every command?  presumably for UDP
         * mode.  */
        if (!settings.socketpath) {
            c->request_addr_size = sizeof(c->request_addr);
        } else {
            c->request_addr_size = 0;
        }

        res = read(c->sfd, c->rbuf + c->rbytes, c->rsize - c->rbytes);
        if (res > 0) {
            stats.bytes_read += res;
            gotdata = 1;
            c->rbytes += res;
            continue;
        }
        if (res == 0) {
            /* connection closed */
            conn_set_state(c, conn_closing);
            return 1;
        }
        if (res == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            else return 0;
        }
    }
    return gotdata;
}

int update_event(conn *c, int new_flags) {
    if (c->ev_flags == new_flags)
        return 1;
    c->ev_flags = new_flags;
    if(aeCreateFileEvent(g_el, c->sfd, AE_READABLE, event_handler, (void *)c) == AE_ERR) {
        return 0;
    }
    return 1;
}

/*
 * Transmit the next chunk of data from our list of msgbuf structures.
 *
 * Returns:
 *   TRANSMIT_COMPLETE   All done writing.
 *   TRANSMIT_INCOMPLETE More data remaining to write.
 *   TRANSMIT_SOFT_ERROR Can't write any more right now.
 *   TRANSMIT_HARD_ERROR Can't write (c->state is set to conn_closing)
 */
int transmit(conn *c) {
    int res;

    if (c->msgcurr < c->msgused &&
            c->msglist[c->msgcurr].msg_iovlen == 0) {
        /* Finished writing the current msg; advance to the next. */
        c->msgcurr++;
    }
    if (c->msgcurr < c->msgused) {
        struct msghdr *m = &c->msglist[c->msgcurr];
        res = sendmsg(c->sfd, m, 0);
        if (res > 0) {
            stats.bytes_written += res;

            /* We've written some of the data. Remove the completed
               iovec entries from the list of pending writes. */
            while (m->msg_iovlen > 0 && res >= m->msg_iov->iov_len) {
                res -= m->msg_iov->iov_len;
                m->msg_iovlen--;
                m->msg_iov++;
            }

            /* Might have written just part of the last iovec entry;
               adjust it so the next write will do the rest. */
            if (res > 0) {
                m->msg_iov->iov_base += res;
                m->msg_iov->iov_len -= res;
            }
            return TRANSMIT_INCOMPLETE;
        }
        if (res == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            if (!update_event(c, AE_WRITABLE)) {
                LOG_INFO("Couldn't update event\n");
                conn_set_state(c, conn_closing);
                return TRANSMIT_HARD_ERROR;
            }
            return TRANSMIT_SOFT_ERROR;
        }
        /* if res==0 or res==-1 and error is not EAGAIN or EWOULDBLOCK,
           we have a real error, on which we close the connection */
        LOG_INFO("Failed to write, and not due to blocking\n");

        if (c->udp)
            conn_set_state(c, conn_read);
        else
            conn_set_state(c, conn_closing);
        return TRANSMIT_HARD_ERROR;
    } else {
        return TRANSMIT_COMPLETE;
    }
}

void drive_machine(conn *c) {

    int exit = 0;
    int sfd, flags = 1;
    socklen_t addrlen;
    struct sockaddr addr;
    conn *newc;
    int res;

    while (!exit) {
        switch(c->state) {
        case conn_listening:
            addrlen = sizeof(addr);
            if ((sfd = accept(c->sfd, &addr, &addrlen)) == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    exit = 1;
                    break;
                } else {
                    perror("accept()");
                }
                break;
            }
            if ((flags = fcntl(sfd, F_GETFL, 0)) < 0 ||
                fcntl(sfd, F_SETFL, flags | O_NONBLOCK) < 0) {
                perror("setting O_NONBLOCK");
                close(sfd);
                break;
            }
            newc = conn_new(sfd, conn_read, AE_READABLE, DATA_BUFFER_SIZE, 0);
            if (!newc) {
                LOG_INFO("Couldn't create new connection\n");
                close(sfd);
                break;
            }

            break;

        case conn_read:
            if (try_read_command(c)) {
                continue;
            }
            if (c->udp ? try_read_udp(c) : try_read_network(c)) {
                continue;
            }
            /* we have no command line and no data to read from network */
            if (!update_event(c, AE_READABLE)) {
                LOG_INFO("Couldn't update event\n");
                conn_set_state(c, conn_closing);
                break;
            }
            exit = 1;
            break;

        case conn_nread:
            /* we are reading rlbytes into ritem; */
            if (c->rlbytes == 0) {
                complete_nread(c);
                break;
            }
            /* first check if we have leftovers in the conn_read buffer */
            if (c->rbytes > 0) {
                int tocopy = c->rbytes > c->rlbytes ? c->rlbytes : c->rbytes;
                memcpy(c->ritem, c->rcurr, tocopy);
                c->ritem += tocopy;
                c->rlbytes -= tocopy;
                c->rcurr += tocopy;
                c->rbytes -= tocopy;
                break;
            }

            /*  now try reading from the socket */
            res = read(c->sfd, c->ritem, c->rlbytes);
            if (res > 0) {
                stats.bytes_read += res;
                c->ritem += res;
                c->rlbytes -= res;
                break;
            }
            if (res == 0) { /* end of stream */
                conn_set_state(c, conn_closing);
                break;
            }
            if (res == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                if (!update_event(c, AE_READABLE)) {
                    LOG_INFO("Couldn't update event\n");
                    conn_set_state(c, conn_closing);
                    break;
                }
                exit = 1;
                break;
            }
            /* otherwise we have a real error, on which we close the connection */
            LOG_INFO("Failed to read, and not due to blocking\n");
            conn_set_state(c, conn_closing);
            break;

        case conn_swallow:
            /* we are reading sbytes and throwing them away */
            if (c->sbytes == 0) {
                conn_set_state(c, conn_read);
                break;
            }

            /* first check if we have leftovers in the conn_read buffer */
            if (c->rbytes > 0) {
                int tocopy = c->rbytes > c->sbytes ? c->sbytes : c->rbytes;
                c->sbytes -= tocopy;
                c->rcurr += tocopy;
                c->rbytes -= tocopy;
                break;
            }

            /*  now try reading from the socket */
            res = read(c->sfd, c->rbuf, c->rsize > c->sbytes ? c->sbytes : c->rsize);
            if (res > 0) {
                stats.bytes_read += res;
                c->sbytes -= res;
                break;
            }
            if (res == 0) { /* end of stream */
                conn_set_state(c, conn_closing);
                break;
            }
            if (res == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                if (!update_event(c, AE_READABLE)) {
                    LOG_INFO("Couldn't update event\n");
                    conn_set_state(c, conn_closing);
                    break;
                }
                exit = 1;
                break;
            }
            /* otherwise we have a real error, on which we close the connection */
            LOG_INFO("Failed to read, and not due to blocking\n");
            conn_set_state(c, conn_closing);
            break;

        case conn_write:
            /*
             * We want to write out a simple response. If we haven't already,
             * assemble it into a msgbuf list (this will be a single-entry
             * list for TCP or a two-entry list for UDP).
             */
            if (c->iovused == 0) {
                if (add_iov(c, c->wcurr, c->wbytes) ||
                        (c->udp && build_udp_headers(c))) {
                    LOG_INFO("Couldn't build response\n");
                    conn_set_state(c, conn_closing);
                    break;
                }
            }

            /* fall through... */

        case conn_mwrite:
            switch (transmit(c)) {
            case TRANSMIT_COMPLETE:
                if (c->state == conn_mwrite) {
                    while (c->ileft > 0) {
                        item *it = *(c->icurr);
                        assert((it->it_flags & ITEM_SLABBED) == 0);
                        item_remove(it);
                        c->icurr++;
                        c->ileft--;
                    }
                    conn_set_state(c, conn_read);
                } else if (c->state == conn_write) {
                    if (c->write_and_free) {
                        free(c->write_and_free);
                        c->write_and_free = 0;
                    }
                    conn_set_state(c, c->write_and_go);
                } else {
                    LOG_INFO_F1("Unexpected state %d\n", c->state);
                    conn_set_state(c, conn_closing);
                }
                break;

            case TRANSMIT_INCOMPLETE:
            case TRANSMIT_HARD_ERROR:
                break;                   /* Continue in state machine. */

            case TRANSMIT_SOFT_ERROR:
                exit = 1;
                break;
            }
            break;

        case conn_closing:
            if (c->udp)
                conn_cleanup(c);
            else
                conn_close(c);
            exit = 1;
            break;
        }

    }

    return;
}

void event_handler(aeEventLoop *el, int fd, void *privdata, int mask) {
    AE_NOTUSED(el); 
    AE_NOTUSED(mask); 

    conn *c;
    c = (conn *)privdata;
    /*
     * c->which = which;
     */

    /* sanity */
    if (fd != c->sfd) {
        LOG_INFO("Catastrophic: event fd doesn't match conn fd!\n");
        conn_close(c);
        return;
    }

    /* do as much I/O as possible until we block */
    drive_machine(c);

    /* wait for next event */
    return;
}

int new_socket(int is_udp) {
    int sfd;
    int flags;

    if ((sfd = socket(AF_INET, is_udp ? SOCK_DGRAM : SOCK_STREAM, 0)) == -1) {
        perror("socket()");
        return -1;
    }

    if ((flags = fcntl(sfd, F_GETFL, 0)) < 0 ||
        fcntl(sfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("setting O_NONBLOCK");
        close(sfd);
        return -1;
    }
    return sfd;
}


/*
 * Sets a socket's send buffer size to the maximum allowed by the system.
 */
void maximize_sndbuf(int sfd) {
    socklen_t intsize = sizeof(int);
    int last_good;
    int min, max, avg;
    int old_size;

    /* Start with the default size. */
    if (getsockopt(sfd, SOL_SOCKET, SO_SNDBUF, &old_size, &intsize)) {
        if (settings.verbose > 0)
            perror("getsockopt(SO_SNDBUF)");
        return;
    }

    /* Binary-search for the real maximum. */
    min = old_size;
    max = MAX_SENDBUF_SIZE;

    while (min <= max) {
        avg = ((unsigned int) min + max) / 2;
        if (setsockopt(sfd, SOL_SOCKET, SO_SNDBUF, &avg, intsize) == 0) {
            last_good = avg;
            min = avg + 1;
        } else {
            max = avg - 1;
        }
    }

    LOG_DEBUG_F3("<%d send buffer was %d, now %d\n", sfd, old_size, last_good);
}


int server_socket(int port, int is_udp) {
    int sfd;
    struct linger ling = {0, 0};
    struct sockaddr_in addr;
    int flags =1;

    if ((sfd = new_socket(is_udp)) == -1) {
        return -1;
    }

    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(flags));
    if (is_udp) {
        maximize_sndbuf(sfd);
    } else {
        setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, &flags, sizeof(flags));
        setsockopt(sfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));
        setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, &flags, sizeof(flags));
    }

    /*
     * the memset call clears nonstandard fields in some impementations
     * that otherwise mess things up.
     */
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = settings.interface;
    if (bind(sfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("bind()");
        close(sfd);
        return -1;
    }
    if (! is_udp && listen(sfd, 1024) == -1) {
        perror("listen()");
        close(sfd);
        return -1;
    }
    return sfd;
}

int new_socket_unix(void) {
    int sfd;
    int flags;

    if ((sfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket()");
        return -1;
    }

    if ((flags = fcntl(sfd, F_GETFL, 0)) < 0 ||
        fcntl(sfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("setting O_NONBLOCK");
        close(sfd);
        return -1;
    }
    return sfd;
}

int server_socket_unix(char *path) {
    int sfd;
    struct linger ling = {0, 0};
    struct sockaddr_un addr;
    struct stat tstat;
    int flags =1;

    if (!path) {
        return -1;
    }

    if ((sfd = new_socket_unix()) == -1) {
        return -1;
    }

    /*
     * Clean up a previous socket file if we left it around
     */
    if (!lstat(path, &tstat)) {
        if (S_ISSOCK(tstat.st_mode))
            unlink(path);
    }

    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(flags));
    setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, &flags, sizeof(flags));
    setsockopt(sfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));

    /*
     * the memset call clears nonstandard fields in some impementations
     * that otherwise mess things up.
     */
    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);
    if (bind(sfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("bind()");
        close(sfd);
        return -1;
    }
    if (listen(sfd, 1024) == -1) {
        perror("listen()");
        close(sfd);
        return -1;
    }
    return sfd;
}


/* invoke right before gdb is called, on assert */
void pre_gdb () {
    int i = 0;
    if(l_socket) close(l_socket);
    if(u_socket > -1) close(u_socket);
    for (i=3; i<=500; i++) close(i); /* so lame */
    kill(getpid(), SIGABRT);
}

int clock_handler(struct aeEventLoop *eventLoop, long long id, void *clientData) {
    set_current_time(&stats);
    return 1000;
}

int timer_delete_handler(struct aeEventLoop *eventLoop, long long id, void *clientData) {
    int i, j=0;
    for (i=0; i<delcurr; i++) {
        item *it = todelete[i];
        if (item_delete_lock_over(it)) {
            assert(it->refcount > 0);
            it->it_flags &= ~ITEM_DELETED;
            item_unlink(it);
            item_remove(it);
        } else {
            todelete[j++] = it;
        }
    }
    delcurr = j;
    return 1000;
}


int l_socket=0;
int u_socket=-1;

void sig_handler(int sig) {
    printf("SIGINT handled.\n");
    exit(0);
}


int main (int argc, char **argv) {
    conn *l_conn;
    /*conn *u_conn;*/
    struct passwd *pw;
    struct sigaction sa;
    struct rlimit rlim;
    
    /* set stderr non-buffering (for running under, say, daemontools) */
    setbuf(stderr, NULL);

    /* handle SIGINT */
    signal(SIGINT, sig_handler);

    /* init settings */
    settings_init(&settings);
    
    /* create AE event loop  */
    g_el = aeCreateEventLoop(AE_SETSIZE);

     
    /* process arguments */
    process_arguments(&settings, argc, argv);

    if (settings.maxcore) {
        struct rlimit rlim_new;
        /*
         * First try raising to infinity; if that fails, try bringing
         * the soft limit to the hard.
         */
        if (getrlimit(RLIMIT_CORE, &rlim)==0) {
            rlim_new.rlim_cur = rlim_new.rlim_max = RLIM_INFINITY;
            if (setrlimit(RLIMIT_CORE, &rlim_new)!=0) {
                /* failed. try raising just to the old max */
                rlim_new.rlim_cur = rlim_new.rlim_max =
                    rlim.rlim_max;
                (void) setrlimit(RLIMIT_CORE, &rlim_new);
            }
        }
        /*
         * getrlimit again to see what we ended up with. Only fail if
         * the soft limit ends up 0, because then no core files will be
         * created at all.
         */

        if ((getrlimit(RLIMIT_CORE, &rlim)!=0) || rlim.rlim_cur==0) {
            fprintf(stderr, "failed to ensure corefile creation\n");
            exit(1);
        }
    }

    /*
     * If needed, increase rlimits to allow as many connections
     * as needed.
     */

    if (getrlimit(RLIMIT_NOFILE, &rlim) != 0) {
        fprintf(stderr, "failed to getrlimit number of files\n");
        exit(1);
    } else {
        int maxfiles = settings.maxconns;
        if (rlim.rlim_cur < maxfiles)
            rlim.rlim_cur = maxfiles + 3;
        if (rlim.rlim_max < rlim.rlim_cur)
            rlim.rlim_max = rlim.rlim_cur;
        if (setrlimit(RLIMIT_NOFILE, &rlim) != 0) {
            fprintf(stderr, "failed to set rlimit for open files. Try running as root or requesting smaller maxconns value.\n");
            exit(1);
        }
    }

    /*
     * initialization order: first create the listening sockets
     * (may need root on low ports), then drop root if needed,
     * then daemonise if needed, then init libevent (in some cases
     * descriptors created by libevent wouldn't survive forking).
     */

    /* create the listening socket and bind it */
    if (!settings.socketpath) {
        l_socket = server_socket(settings.port, 0);
        if (l_socket == -1) {
            fprintf(stderr, "failed to listen\n");
            exit(1);
        }
    }

    if (settings.udpport > 0 && ! settings.socketpath) {
        /* create the UDP listening socket and bind it */
        u_socket = server_socket(settings.udpport, 1);
        if (u_socket == -1) {
            fprintf(stderr, "failed to listen on UDP port %d\n", settings.udpport);
            exit(1);
        }
    }

    /* lose root privileges if we have them */
    if (getuid()== 0 || geteuid()==0) {
        if (settings.username==0 || *(settings.username)=='\0') {
            fprintf(stderr, "can't run as root without the -u switch\n");
            return 1;
        }
        if ((pw = getpwnam(settings.username)) == 0) {
            fprintf(stderr, "can't find the user %s to switch to\n", settings.username);
            return 1;
        }
        if (setgid(pw->pw_gid)<0 || setuid(pw->pw_uid)<0) {
            fprintf(stderr, "failed to assume identity of user %s\n", settings.username);
            return 1;
        }
    }

    /* create unix mode sockets after dropping privileges */
    if (settings.socketpath) {
        l_socket = server_socket_unix(settings.socketpath);
        if (l_socket == -1) {
            fprintf(stderr, "failed to listen\n");
            exit(1);
        }
    }

    /* daemonize if requested */
    /* if we want to ensure our ability to dump core, don't chdir to / */
    if (settings.daemonize) {
        int res;
        res = daemon(settings.maxcore, settings.verbose);
        if (res == -1) {
            fprintf(stderr, "failed to daemon() in order to daemonize\n");
            return 1;
        }
    }


    /* initialize other stuff */
    item_init();
    stats_init(&stats);
    assoc_init();
    conn_init();
    slabs_init(settings.maxbytes, settings.factor);

    /* managed instance? alloc and zero a bucket array */
    if (settings.managed) {
        buckets = malloc(sizeof(int)*MAX_BUCKETS);
        if (buckets == 0) {
            fprintf(stderr, "failed to allocate the bucket array");
            exit(1);
        }
        memset(buckets, 0, sizeof(int)*MAX_BUCKETS);
    }

    /* lock paged memory if needed */
    if (settings.lock_memory) {
        mlockall(MCL_CURRENT | MCL_FUTURE);
        fprintf(stderr, "warning: mlockall() not supported on this platform.  proceeding without.\n");
    }

    /*
     * ignore SIGPIPE signals; we can use errno==EPIPE if we
     * need that information
     */
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigemptyset(&sa.sa_mask) == -1 ||
        sigaction(SIGPIPE, &sa, 0) == -1) {
        perror("failed to ignore SIGPIPE; sigaction");
        exit(1);
    }
    /* create the initial listening connection */
    if (!(l_conn = conn_new(l_socket, conn_listening, AE_READABLE, 1, 0))) {
        fprintf(stderr, "failed to create listening connection");
        exit(1);
    }
    /* create the initial listening udp connection     
      if (u_socket > -1 &&
        !(u_conn = conn_new(u_socket, conn_read, EV_READ | EV_PERSIST, UDP_READ_BUFFER_SIZE, 1))) {
        fprintf(stderr, "failed to create udp connection");
        exit(1);
    }*/
    /* initialise clock event */
    aeCreateTimeEvent(g_el, 1000, clock_handler, NULL, NULL);
    /* initialise deletion array and timer event */
    deltotal = 200; delcurr = 0;
    todelete = malloc(sizeof(item *)*deltotal);
    aeCreateTimeEvent(g_el, 1000, timer_delete_handler, NULL, NULL);
    
    /* create command service */
    g_cmd_srv = command_service_create();
    /* register commands */
    /* reading commands */
    command_service_register_handler(g_cmd_srv, "get ", 4, FIXED_PREFIX, get_handler);
    command_service_register_handler(g_cmd_srv, "bget ", 5, FIXED_PREFIX, bget_handler);

    /* writting commands */
    command_service_register_handler(g_cmd_srv, "set ", 4, FIXED_PREFIX, set_handler);
    command_service_register_handler(g_cmd_srv, "add ", 4, FIXED_PREFIX, add_handler);
    command_service_register_handler(g_cmd_srv, "replace ", 8, FIXED_PREFIX, replace_handler);
    command_service_register_handler(g_cmd_srv, "incr ", 5, FIXED_PREFIX, incr_handler);
    command_service_register_handler(g_cmd_srv, "decr ", 5, FIXED_PREFIX, decr_handler);
    command_service_register_handler(g_cmd_srv, "delete ", 7, FIXED_PREFIX, delete_handler);
    
    command_service_register_handler(g_cmd_srv, "own ", 4, FIXED_PREFIX, own_handler);
    command_service_register_handler(g_cmd_srv, "disown ", 7, FIXED_PREFIX, disown_handler);

    /* statistics commands */
    command_service_register_handler(g_cmd_srv, "stats", 5, FULL_MATCH, stats_handler);
    command_service_register_handler(g_cmd_srv, "stats reset", 11, FULL_MATCH, stats_reset_handler);
    command_service_register_handler(g_cmd_srv, "stats malloc", 12, FULL_MATCH, stats_malloc_handler);
    command_service_register_handler(g_cmd_srv, "stats maps", 10, FULL_MATCH, stats_maps_handler);
    command_service_register_handler(g_cmd_srv, "stats cachedump", 15, FULL_MATCH, stats_cachedump_handler);
    command_service_register_handler(g_cmd_srv, "stats slabs", 11, FULL_MATCH, stats_slabs_handler);
    command_service_register_handler(g_cmd_srv, "stats items", 11, FULL_MATCH, stats_items_handler);
    command_service_register_handler(g_cmd_srv, "stats sizes", 11, FULL_MATCH, stats_sizes_handler);
    
    /* other commands */
    command_service_register_handler(g_cmd_srv, "version", 7, FULL_MATCH, version_handler);
    command_service_register_handler(g_cmd_srv, "quit", 4, FULL_MATCH, quit_handler);
    command_service_register_handler(g_cmd_srv, "slabs reassign ", 15, FIXED_PREFIX, quit_handler);
    command_service_register_handler(g_cmd_srv, "flush_all", 9, FIXED_PREFIX, flush_all_handler);
    command_service_register_handler(g_cmd_srv, "bg ", 3, FIXED_PREFIX, bg_handler);

    /* save the PID in if we're a daemon */
    if (settings.daemonize && NULL != settings.pid_file) {
        FILE *fp;
        if (!(fp = fopen(settings.pid_file,"w"))) {
            fprintf(stderr,"Could not open the pid file %s for writing\n",settings.pid_file);
        } else {
            fprintf(fp,"%ld\n",(long) getpid());
            if (fclose(fp) < 0) {
                fprintf(stderr,"Could not close the pid file %s.\n",settings.pid_file);
            }
        }
    }

    /* enter the loop */
    aeMain(g_el);
    aeDeleteEventLoop(g_el);
    
    /* destory command service */
    command_service_destory(g_cmd_srv);

    /* remove the PID file if we're a daemon */
    if (settings.daemonize && NULL != settings.pid_file) {
        if (unlink(settings.pid_file) < 0) {
            fprintf(stderr,"Could not remove the pid file %s.\n",settings.pid_file);
        }
    }
    return 0;
}
