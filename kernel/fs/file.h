#ifndef FS_FILE_H
#define FS_FILE_H

#include "node.h"
#include "uint64.h"

struct FileHandle
{
	struct Handle super;

	Uint64 position;
};

struct File* file_create_node(char* name, unsigned int flags, struct FileOps* ops);

struct FileHandle* file_open(struct Node* file, unsigned int mode);
void file_close(struct FileHandle* handle);

#define FILE_IO_RESULT_STATUS_MASK 0xf000
#define FILE_IO_RESULT_SIZE_MASK 0x0fff

#define FILE_IO_STATUS_SUCCESS 0
#define FILE_IO_STATUS_ERROR 0x1000

unsigned int file_read(struct FileHandle* handle, char* buffer, unsigned int size);
unsigned int file_write(struct FileHandle* handle, char* buffer, unsigned int size);

void file_seek(struct FileHandle* handle, Uint64* position);

typedef struct FileHandle* (FileOpen)(struct File* file, unsigned int mode);
typedef unsigned int (FileRead)(struct FileHandle* handle, char*, unsigned int);
typedef unsigned int (FileWrite)(struct FileHandle* handle, char*, unsigned int);

struct FileOps
{
	FileOpen* open;
	FileRead* read;
	FileWrite* write;
};

struct File
{
	struct Node super;

	struct FileOps* ops;
};

#endif

