#ifndef SYS_H
#define SYS_H

#include "int64.h"

unsigned int sys_open(char* path);
void sys_close(unsigned int handle);
void sys_exit(unsigned int code);

unsigned int sys_read(unsigned int handle, char* buffer, struct Int64* position, unsigned int size);
unsigned int sys_write(unsigned int handle, char* buffer, struct Int64* position, unsigned int size);

unsigned int sys_sleep(unsigned int milliseconds);

#endif
