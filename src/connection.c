#include <stdio.h>
#include <stdlib.h>
#include "ae.h"
#include "connection.h"
#include "memcached.h"

extern struct stats stats;
extern aeEventLoop *g_el;

conn **freeconns;
int freetotal;
int freecurr;

void conn_init(void) {
    freetotal = 200;
    freecurr = 0;
    freeconns = (conn **)malloc(sizeof (conn *)*freetotal);
    return;
}

conn *conn_new(int sfd, int init_state, int event_flags, int read_buffer_size,
                int is_udp) {
    conn *c;

    /* do we have a free conn structure from a previous close? */
    if (freecurr > 0) {
        c = freeconns[--freecurr];
    } else { /* allocate a new one */
        if (!(c = (conn *)malloc(sizeof(conn)))) {
            perror("malloc()");
            return 0;
        }
        c->rbuf = c->wbuf = 0;
        c->ilist = 0;
        c->iov = 0;
        c->msglist = 0;
        c->hdrbuf = 0;

        c->rsize = read_buffer_size;
        c->wsize = DATA_BUFFER_SIZE;
        c->isize = ITEM_LIST_INITIAL;
        c->iovsize = IOV_LIST_INITIAL;
        c->msgsize = MSG_LIST_INITIAL;
        c->hdrsize = 0;

        c->rbuf = (char *) malloc(c->rsize);
        c->wbuf = (char *) malloc(c->wsize);
        c->ilist = (item **) malloc(sizeof(item *) * c->isize);
        c->iov = (struct iovec *) malloc(sizeof(struct iovec) * c->iovsize);
        c->msglist = (struct msghdr *) malloc(sizeof(struct msghdr) * c->msgsize);

        if (c->rbuf == 0 || c->wbuf == 0 || c->ilist == 0 || c->iov == 0 ||
                c->msglist == 0) {
            if (c->rbuf != 0) free(c->rbuf);
            if (c->wbuf != 0) free(c->wbuf);
            if (c->ilist !=0) free(c->ilist);
            if (c->iov != 0) free(c->iov);
            if (c->msglist != 0) free(c->msglist);
            free(c);
            perror("malloc()");
            return 0;
        }

        stats.conn_structs++;
    }

    if (settings.verbose > 1) {
        if (init_state == conn_listening)
            fprintf(stderr, "<%d server listening\n", sfd);
        else if (is_udp)
            fprintf(stderr, "<%d server listening (udp)\n", sfd);
        else
            fprintf(stderr, "<%d new client connection\n", sfd);
    }

    c->sfd = sfd;
    c->udp = is_udp;
    c->state = init_state;
    c->rlbytes = 0;
    c->rbytes = c->wbytes = 0;
    c->wcurr = c->wbuf;
    c->rcurr = c->rbuf;
    c->ritem = 0;
    c->icurr = c->ilist;
    c->ileft = 0;
    c->iovused = 0;
    c->msgcurr = 0;
    c->msgused = 0;

    c->write_and_go = conn_read;
    c->write_and_free = 0;
    c->item = 0;
    c->bucket = -1;
    c->gen = 0;

    //event_set(&c->event, sfd, event_flags, event_handler, (void *)c);
    c->ev_flags = event_flags;

    //if (event_add(&c->event, 0) == -1) {
    if (aeCreateFileEvent(g_el, sfd, event_flags, event_handler, (void *)c) == AE_ERR) {
        if (freecurr < freetotal) {
            freeconns[freecurr++] = c;
        } else {
            if (c->hdrbuf)
                free (c->hdrbuf);
            free (c->msglist);
            free (c->rbuf);
            free (c->wbuf);
            free (c->ilist);
            free (c->iov);
            free (c);
        }
        return 0;
    }

    stats.curr_conns++;
    stats.total_conns++;

    return c;
}

void conn_cleanup(conn *c) {
    if (c->item) {
        item_free(c->item);
        c->item = 0;
    }

    if (c->ileft) {
        for (; c->ileft > 0; c->ileft--,c->icurr++) {
            item_remove(*(c->icurr));
        }
    }

    if (c->write_and_free) {
        free(c->write_and_free);
        c->write_and_free = 0;
    }
}

/*
 * Frees a connection.
 */
static void conn_free(conn *c) {
    if (c) {
        if (c->hdrbuf)
            free(c->hdrbuf);
        if (c->msglist)
            free(c->msglist);
        if (c->rbuf)
            free(c->rbuf);
        if (c->wbuf)
            free(c->wbuf);
        if (c->ilist)
            free(c->ilist);
        if (c->iov)
            free(c->iov);
        free(c);
    }
}

void conn_close(conn *c) {
    /* delete the event, the socket and the conn */
    //event_del(&c->event);
    aeDeleteFileEvent(g_el, c->sfd, AE_READABLE); 
    aeDeleteFileEvent(g_el, c->sfd, AE_WRITABLE);
    if (settings.verbose > 1)
        fprintf(stderr, "<%d connection closed.\n", c->sfd);

    close(c->sfd);
    conn_cleanup(c);

    /* if the connection has big buffers, just free it */
    if (c->rsize > READ_BUFFER_HIGHWAT) {
        conn_free(c);
    } else if (freecurr < freetotal) {
        /* if we have enough space in the free connections array, put the structure there */
        freeconns[freecurr++] = c;
    } else {
        /* try to enlarge free connections array */
        conn **new_freeconns = realloc(freeconns, sizeof(conn *)*freetotal*2);
        if (new_freeconns) {
            freetotal *= 2;
            freeconns = new_freeconns;
            freeconns[freecurr++] = c;
        } else {
            conn_free(c);
        }
    }

    stats.curr_conns--;

    return;
}



/*
 * Reallocates memory and updates a buffer size if successful.
 */
int do_realloc(void **orig, int newsize, int bytes_per_item, int *size) {
    void *newbuf = realloc(*orig, newsize * bytes_per_item);
    if (newbuf) {
        *orig = newbuf;
        *size = newsize;
       return 1;
    }
    return 0;
}

 /*
 * Shrinks a connection's buffers if they're too big.  This prevents
 * periodic large "get" requests from permanently chewing lots of server
 * memory.
 *
 * This should only be called in between requests since it can wipe output
 * buffers!
 */
void conn_shrink(conn *c) {
    if (c->udp)
        return;

    if (c->rsize > READ_BUFFER_HIGHWAT && c->rbytes < DATA_BUFFER_SIZE) {
       do_realloc((void **)&c->rbuf, DATA_BUFFER_SIZE, 1, &c->rsize);
    }

    if (c->isize > ITEM_LIST_HIGHWAT) {
        do_realloc((void **)&c->ilist, ITEM_LIST_INITIAL, sizeof(c->ilist[0]), &c->isize);
    }

    if (c->msgsize > MSG_LIST_HIGHWAT) {
        do_realloc((void **)&c->msglist, MSG_LIST_INITIAL, sizeof(c->msglist[0]), &c->msgsize);
    }

    if (c->iovsize > IOV_LIST_HIGHWAT) {
        do_realloc((void **)&c->iov, IOV_LIST_INITIAL, sizeof(c->iov[0]), &c->iovsize);
    }
}

/*
 * Sets a connection's current state in the state machine. Any special
 * processing that needs to happen on certain state transitions can
 * happen here.
 */
void conn_set_state(conn *c, int state) {
    if (state != c->state) {
        if (state == conn_read) {
            conn_shrink(c);
        }
        c->state = state;
    }
}


