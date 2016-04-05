#ifndef CONNECTION_H
#define CONNECTION_H

#include "items.h"

typedef struct {
    int    sfd;
    int    state;
    struct aeEventLoop *event;
    short  ev_flags;
    short  which;   /* which events were just triggered */

    char   *rbuf;   /* buffer to read commands into */
    char   *rcurr;  /* but if we parsed some already, this is where we stopped */
    int    rsize;   /* total allocated size of rbuf */
    int    rbytes;  /* how much data, starting from rcur, do we have unparsed */

    char   *wbuf;
    char   *wcurr;
    int    wsize;
    int    wbytes;
    int    write_and_go; /* which state to go into after finishing current write */
    void   *write_and_free; /* free this memory after finishing writing */

    char   *ritem;  /* when we read in an item's value, it goes here */
    int    rlbytes;

    /* data for the nread state */

    /*
     * item is used to hold an item structure created after reading the command
     * line of set/add/replace commands, but before we finished reading the actual
     * data. The data is read into ITEM_data(item) to avoid extra copying.
     */

    void   *item;     /* for commands set/add/replace  */
    int    item_comm; /* which one is it: set/add/replace */

    /* data for the swallow state */
    int    sbytes;    /* how many bytes to swallow */

    /* data for the mwrite state */
    struct iovec *iov;
    int    iovsize;   /* number of elements allocated in iov[] */
    int    iovused;   /* number of elements used in iov[] */

    struct msghdr *msglist;
    int    msgsize;   /* number of elements allocated in msglist[] */
    int    msgused;   /* number of elements used in msglist[] */
    int    msgcurr;   /* element in msglist[] being transmitted now */
    int    msgbytes;  /* number of bytes in current msg */

    item   **ilist;   /* list of items to write out */
    int    isize;
    item   **icurr;
    int    ileft;

    /* data for UDP clients */
    int    udp;       /* 1 if this is a UDP "connection" */
    int    request_id; /* Incoming UDP request ID, if this is a UDP "connection" */
    struct sockaddr request_addr; /* Who sent the most recent request */
    socklen_t request_addr_size;
    unsigned char *hdrbuf; /* udp packet headers */
    int    hdrsize;   /* number of headers' worth of space is allocated */

    int    binary;    /* are we in binary mode */
    int    bucket;    /* bucket number for the next command, if running as
                         a managed instance. -1 (_not_ 0) means invalid. */
    int    gen;       /* generation requested for the bucket */
} conn;

conn *conn_new(int sfd, int init_state, int event_flags, int read_buffer_size, int is_udp);
void conn_close(conn *c);
void conn_init(void);

#endif // end definition of CONNECTION_H
