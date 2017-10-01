#include "io.h"
#include "memory.h"
#include "string.h"
#include "sys.h"
#include "uint64.h"
#include "printf.h"

extern struct FileHandle* output;

int rmmtosMain(char* arg)
{
	sys_unmount();

	return 0;
}

