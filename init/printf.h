#ifndef __PRINTF_H
#define __PRINTF_H

#include "io.h"

void putch_screen(struct FileHandle* h);
void printf(char* fmt,...);
void puts(char* str);

#endif
