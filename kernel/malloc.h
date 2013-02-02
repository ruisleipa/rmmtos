#ifndef __MALLOC_H
#define __MALLOC_H

void* malloc(unsigned int size);
void free(void* block);
void malloc_init(void* start,void* end);
void malloc_info();
void malloc_info_terse();

#endif

