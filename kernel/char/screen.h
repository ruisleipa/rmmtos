#ifndef __SCREEN_H
#define __SCREEN_H

#include "fs/node.h"

void screen_init();
void screen_init_device();
void screen_putch(char c);
unsigned int screen_write(struct FileHandle* handle, char* buffer, FilePosition position, unsigned int size);

#endif


