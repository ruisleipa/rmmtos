#ifndef __IO_H
#define __IO_H

struct FilePosition
{
	struct
	{
		unsigned int lo;
		unsigned int hi;
	}lo;

	struct
	{
		unsigned int lo;
		unsigned int hi;
	}hi;
};

struct FileHandle;

#define READ 0x0001
#define WRITE 0x0002

struct FileHandle* openFile(char* path, unsigned int mode);
void closeFile(struct FileHandle* handle);

unsigned int seekFile(struct FileHandle* handle, struct FilePosition* position);
unsigned int readFile(struct FileHandle* handle, char* buffer, unsigned int count);
unsigned int writeFile(struct FileHandle* handle, char* buffer, unsigned int count);

#endif
