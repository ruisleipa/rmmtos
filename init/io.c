#include "io.h"
#include "memory.h"
#include "sys.h"

struct FileHandle
{
	unsigned int handle;
};

struct FileHandle* openFile(char* path, unsigned int mode)
{
	unsigned int h = sys_open_file(path, mode);
	struct FileHandle* handle;

	if(!h)
		return 0;

	handle = allocateMemory(sizeof(struct FileHandle));

	if(!handle)
		return 0;

	handle->handle = h;

	return handle;
}

void closeFile(struct FileHandle* handle)
{
	sys_close_file(handle->handle);

	freeMemory(handle);
}

unsigned int readFile(struct FileHandle* handle, char* buffer, unsigned int count)
{
	sys_read_file(handle->handle, buffer, count);
}

unsigned int writeFile(struct FileHandle* handle, char* buffer, unsigned int count)
{
	sys_write_file(handle->handle, buffer, count);
}
