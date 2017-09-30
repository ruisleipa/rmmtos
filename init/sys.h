#ifndef SYS_H
#define SYS_H

#include "uint64.h"

unsigned int sys_open(char* path);
unsigned int sys_exec(char* path);
void sys_close(unsigned int handle);
void sys_exit(unsigned int code);

unsigned int sys_read(unsigned int handle, char* buffer, unsigned int size);
unsigned int sys_write(unsigned int handle, char* buffer, unsigned int size);

unsigned int sys_sleep(unsigned int milliseconds);
unsigned int sys_mount(char* destination_path, char* type, char* source_path);

#endif
