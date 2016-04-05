#ifndef ITEMS_H
#define ITEMS_H

#include "config.h"

#define ITEM_LINKED 1
#define ITEM_DELETED 2

/* temp */
#define ITEM_SLABBED 4

typedef struct _stritem {
    struct _stritem *next;
    struct _stritem *prev;
    struct _stritem *h_next;    /* hash chain next */
    rel_time_t      time;       /* least recent access */
    rel_time_t      exptime;    /* expire time */
    int             nbytes;     /* size of data */
    unsigned short  refcount;
    unsigned char   nsuffix;    /* length of flags-and-length string */
    unsigned char   it_flags;   /* ITEM_* above */
    unsigned char   slabs_clsid;/* which slab class we're in */
    unsigned char   nkey;       /* key length, w/terminating null and padding */
    void * end[0];
    /* then null-terminated key */
    /* then " flags length\r\n" (no terminating null) */
    /* then data with terminating \r\n (no terminating null; it's binary!) */
} item;

#define ITEM_key(item) ((char*)&((item)->end[0]))

/* warning: don't use these macros with a function, as it evals its arg twice */
#define ITEM_suffix(item) ((char*) &((item)->end[0]) + (item)->nkey)
#define ITEM_data(item) ((char*) &((item)->end[0]) + (item)->nkey + (item)->nsuffix)
#define ITEM_ntotal(item) (sizeof(struct _stritem) + (item)->nkey + (item)->nsuffix + (item)->nbytes)


void item_init(void);
item *item_alloc(char *key, int flags, rel_time_t exptime, int nbytes);
void item_free(item *it);
int item_size_ok(char *key, int flags, int nbytes);
int item_link(item *it);    /* may fail if transgresses limits */
void item_unlink(item *it);
void item_remove(item *it);
void item_update(item *it);   /* update LRU time to current and reposition */
int item_replace(item *it, item *new_it);
char *item_cachedump(unsigned int slabs_clsid, unsigned int limit, unsigned int *bytes);
char *item_stats_sizes(int *bytes);
void item_stats(char *buffer, int buflen);


#endif // end define ITEMS_H
