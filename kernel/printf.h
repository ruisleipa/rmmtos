#ifndef __PRINTF_H
#define __PRINTF_H

void printf(char* fmt,...);

typedef void (putch_t)(char);

void putch_set(putch_t* func);
void puts(char* str);

#endif
