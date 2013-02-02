#ifndef __MEMORY_H
#define __MEMORY_H

void* allocateMemory(unsigned int size);
void freeMemory(void* block);
void clearMemory(void* ptr, unsigned int size);

#endif

