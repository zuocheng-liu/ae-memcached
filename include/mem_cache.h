#ifndef MEM_CACHE_H
#define MEM_CACHE_H

#include "slab.h"

#define DEFAULT_POWER_SMALLEST 1
#define DEFAULT_POWER_LARGEST  200
#define DEFAULT_POWER_BLOCK 1048576 /* 1M */

/* powers-of-N allocation structures */
typedef struct mem_cache{
    double factor;
    size_t base_chunk_size;
    size_t power_block;
    size_t power_smallest;
    size_t power_largest;
    size_t mem_limit;
    size_t mem_malloced;
    slab_t **slabclass; /* Array of slab pointers, index 0 is reserved. */
} mem_cache_t, *mem_cache_ptr;

/* Init the subsystem. 1st argument is the limit on no. of bytes to allocate,
   0 if no limit. 2nd argument is the growth factor; each slab will use a chunk
   size equal to the previous slab's chunk size times this factor. */
mem_cache_ptr
mem_cache_create(size_t base_chunk_size, 
        double factor, 
        size_t power_largest, 
        size_t power_block, 
        size_t mem_limit, 
        int pre_alloc);

void *mem_cache_alloc(mem_cache_ptr mem_cache, size_t size);

void mem_cache_free(mem_cache_ptr mem_cache, void *ptr, size_t size);

size_t mem_cache_clsid(mem_cache_ptr mem_cache, size_t size);
#endif /* End definition of MEM_CACHE_H */
