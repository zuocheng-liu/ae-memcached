#include "fmalloc.h"
#include "slabs.h"

void *fmalloc(size_t size) {
    return (void *)size;   
}
void *fcalloc(size_t size);
void *frealloc(void *ptr, size_t size);
void ffree(void *ptr);


