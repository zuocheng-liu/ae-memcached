#ifndef SLABS_H
#define SLABS_H
/* slabs memory allocation */

/* Init the subsystem. 1st argument is the limit on no. of bytes to allocate,
   0 if no limit. 2nd argument is the growth factor; each slab will use a chunk
   size equal to the previous slab's chunk size times this factor. */
void slabs_init(size_t limit, double factor);

/* Preallocate as many slab pages as possible (called from slabs_init)
   on start-up, so users don't get confused out-of-memory errors when
   they do have free (in-slab) space, but no space to make new slabs.
   if maxslabs is 18 (POWER_LARGEST - POWER_SMALLEST + 1), then all
   slab types can be made.  if max memory is less than 18 MB, only the
   smaller ones will be made.  */
void slabs_preallocate (unsigned int maxslabs);

/* Given object size, return id to use when allocating/freeing memory for object */
/* 0 means error: can't store such a large object */
unsigned int slabs_clsid(size_t size);

/* Allocate object of given length. 0 on error */
void *slabs_alloc(size_t size);

/* Free previously allocated object */
void slabs_free(void *ptr, size_t size);

/* Fill buffer with stats */
char* slabs_stats(int *buflen);

/* Request some slab be moved between classes
  1 = success
   0 = fail
   -1 = tried. busy. send again shortly. */
int slabs_reassign(unsigned char srcid, unsigned char dstid);

#endif // end define SLABS_H
