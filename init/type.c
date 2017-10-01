#include "io.h"
#include "memory.h"
#include "string.h"
#include "sys.h"
#include "uint64.h"
#include "printf.h"

extern struct FileHandle* output;

int rmmtosMain(char* arg)
{
	struct FileHandle* file = openFile(arg, READ);
	char buf[512];
	unsigned int res;

	do {
		res = readFile(file, buf, 512);
		writeFile(output, buf, res & 0x0fff);
	} while((res & 0x0fff) > 0);

	printf("\n");

	return 0;
}

