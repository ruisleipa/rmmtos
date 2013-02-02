#ifndef __PANIC_H
#define __PANIC_H

void do_panic(char* str,char* file,unsigned int line);

#define panic(str) do_panic(str,__FILE__,__LINE__)

#endif

