#include "io.h"
#include "memory.h"
#include "string.h"
#include "sys.h"
#include "uint64.h"

#define BUFFER (512*2)

void printstr(struct File* f, char* str);

void printbuf(struct File* file, char* buffer, unsigned int size);

extern void* _end;

int main()
{
	struct File* screen;
	struct File* test;
	unsigned int read;

	malloc_init((void*)&_end, (void*)0xfdff);

	screen = openFile("/devices/screen", WRITE);

	printstr(screen, "Mounting drives...\n");
	sys_mount("/floppy", "fat", "/devices/floppy0");

	printstr(screen, "Executing shell...\n");

	closeFile(screen);

	sys_exec("/floppy/shell.bin");

	sys_exit(1);
}
/*
void main() {
	struct File* floppy = openFile("/devices/floppy0", READ);
	struct File* screen = openFile("/devices/screen", WRITE);
	char* buffer = allocateMemory(BUFFER);
	Uint64 pos;

	printstr(screen, "Hello from init!");

	clearMemory(buffer, BUFFER);

	printstr(screen, "Sleeping.");
	sys_sleep(1000);
	printstr(screen, "Slept for 1 seconds.\n");

        printstr(screen, "Mounting floppy:\n");
	//sys_mount("/floppy", "fat", "/devices/floppy0");
	//sys_exit(0);
//		while(1);
	if(!buffer)
	{
		sys_exit(0);
		while(1);
	}

	if(!floppy || !screen)
	{
		sys_exit(1);
		while(1);
	}

        printstr(screen, "The following is read from floppy:\n");

	init64(&pos, 0, 0, 0x1, 0x2200);
	seekFile(floppy, &pos);
	readFile(floppy, buffer, BUFFER);
	printbuf(screen, buffer, BUFFER);

	sys_exit(2);

	while(1);
}
*/

void printbuf(struct File* file, char* buffer, unsigned int size) {
	unsigned int i;
	unsigned char buf[64];

	for(i = 0; i < size; i++)
	{
		if(i % 64 == 0 && i != 0) {
			writeFile(file, buf, 64);
			writeFile(file, "\n", 1);
		}

		if(buffer[i] >= ' ')
			buf[i%64] = buffer[i];
		else
			buf[i%64] = '.';
	}

	writeFile(file, buf, 64);
	writeFile(file, "\n", 1);
}

void printstr(struct File* f, char* str) {
    writeFile(f, str, strlen(str));
}
