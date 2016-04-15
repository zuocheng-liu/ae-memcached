#ifndef FMALLOC_H
#define FMALLOC_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
void *fmalloc(size_t size); 
void *fcalloc(size_t size);
void *frealloc(void *ptr, size_t size);
void ffree(void *ptr);

#endif /* FMALLOC_H */
