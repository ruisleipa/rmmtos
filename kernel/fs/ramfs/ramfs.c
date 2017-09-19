#include "ramfs.h"

#include "fs/directory.h"
#include "fs/file.h"
#include "panic.h"

static struct DirectoryOps ramfs_dir_ops =
{
	0,					/* open */
	0,					/* get_next_node */
	0,					/* find_node */
	0					/* add_node */
};

static struct FileOps ramfs_file_ops =
{
	0, /* open */
	0, /* read */
	0  /* write */
};

struct RamfsFile
{
	struct File super;

	unsigned int size;
	unsigned int alloc_size;
	char* data;
};

struct RamfsFile* ramfs_allocate_file_for_node(struct File* file)
{
	struct RamfsFile* file = 0;
	struct RamfsFile* ramfs_file = realloc(file, sizeof(struct RamfsFile));

	if(!file)
		return 0;

	ramfs_file->size = 0;
	ramfs_file->alloc_size = 0;
	ramfs_file->data = 0;

	return ramfs_file;
}

struct Node* ramfs_directory_add_node(struct Directory* dir, struct Node* node)
{
	if(node->flags & FILE)
	{
		struct File* file;

		file = node;

		if(file->ops)
			panic("ops set");

		file->ops = &ramfs_file_ops;

		return ramfs_allocate_file_for_node(node);
	}

	if(node->flags & DIRECTORY)
	{
		struct Directory* directory;

		directory = node;

		if(directory->ops)
			panic("ops set");

		directory->ops = &ramfs_dir_ops;
	}

	return node;
}


struct Directory* ramfs_create()
{
	return directory_create_node("", STATIC, &ramfs_dir_ops);
}

