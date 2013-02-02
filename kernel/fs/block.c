#include "block.h"
#include "uint64.h"
#include "file.h"

static char* non_file = "file op on non-file";
/*
struct BlockFile
{
	struct File super;

	unsigned int block_size_exponent;
	struct BlockFileOps* ops;
};
*/
static struct FileOps block_ops =
{
	0,

	blockfile_read,
	blockfile_write
};

struct BlockFile* blockfile_create_node(char* name, unsigned int flags, struct BlockFileOps* ops, unsigned int block_size_exponent)
{
	struct File* file = file_create_node(name, flags, &block_ops);
	struct BlockFile* block_file = realloc(file, sizeof(struct BlockFile));

	if(block_file)
	{
		block_file->block_size_exponent = block_size_exponent;
		block_file->ops = ops;
	}
	else
	{
		free(file);
	}

	return block_file;
}

unsigned int blockfile_read(struct FileHandle* handle, char* buffer, unsigned int size)
{
	struct BlockFile* block_file = handle->super.node;
	Uint64 block;
	Uint64 begin_block;
	Uint64 end_block;
	Uint64 t;
	unsigned int part_begin;
	unsigned int part_size;

	unsigned int offset_mask = ~(0xffff << block_file->block_size_exponent);

	unsigned int block_size = 1 << block_file->block_size_exponent;

	char* tmp = 0;

	if(!block_file->ops || !block_file->ops->read)
		return 0;

	set64(&block, &handle->position);
	shl64(&block, block_file->block_size_exponent);

	set64(&begin_block, &block);

	set64(&end_block, &handle->position);
	/* adding (block_size - 1) to get correct end block number rounding */
	init64(&t, 0, 0, 0, size - 1);

	add64(&end_block, &t);
	shl64(&end_block, block_file->block_size_exponent);

	while(cmp64(&block, &end_block) <= 0)
	{
		if(cmp64(&block, &begin_block) == 0)
			part_begin = handle->position.i[0] & offset_mask;
		else
			part_begin = 0;

		if(cmp64(&block, &end_block) == 0)
			part_size = size;
		else
			part_size = block_size;

		part_size -= part_begin;

		if(part_begin != 0 || part_size != block_size)
		{
			if(!tmp)
				tmp = malloc(block_size);

			if(!tmp)
				break;

			block_file->ops->read(handle, tmp, &block);

			memcpy(buffer, tmp + part_begin, part_size);
		}
		else
		{
			block_file->ops->read(handle, buffer, &block);
		}

		buffer += part_size;

		init64(&t, 0, 0, 0, part_size);

		add64(&handle->position, &t);

		inc64(&block);
	}

	if(tmp)
		free(tmp);

	return size;
}

unsigned int blockfile_write(struct FileHandle* file, char* buffer, unsigned int size)
{
	return 0;
}
