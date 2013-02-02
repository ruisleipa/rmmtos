#ifndef FS_BLOCKFILE_H
#define FS_BLOCKFILE_H

#include "node.h"
#include "uint64.h"
#include "file.h"

struct BlockFile
{
	struct File super;

	unsigned int block_size_exponent;
	struct BlockFileOps* ops;
};


struct BlockFile* blockfile_create_node(char* name, unsigned int flags, struct BlockFileOps* ops, unsigned int block_size_exponent);

unsigned int blockfile_read(struct FileHandle* handle, char* buffer, unsigned int size);
unsigned int blockfile_write(struct FileHandle* handle, char* buffer, unsigned int size);

typedef unsigned int (BlockFileRead)(struct FileHandle* handle, char* buffer, struct Uint64* block);
typedef unsigned int (BlockFileWrite)(struct FileHandle* handle, char* buffer, struct Uint64* block);

struct BlockFileOps
{
	BlockFileRead* read;
	BlockFileWrite* write;
};

#endif

