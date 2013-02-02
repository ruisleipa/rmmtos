#include "ramfs.h"

#include "fs/directory.h"
#include "panic.h"

static struct FsNodeOps ramfs_ops =
{
	0,					/* open */
	0,					/* close */

	0,					/* read */
	0,					/* write */

	directory_get_node_count,		/* get_node_count */
	directory_get_node_by_name,		/* get_node_by_name */
	directory_get_node_by_index,		/* get_node_by_index */
	directory_add_node,		/* add_node */
	0					/* remove_node */
};

struct FsNode ramfs_root =
{
	"",					/* name */
	DIRECTORY | STATIC,			/* flags */
	0,					/* id */
	-1,					/* use_count */

	&ramfs_ops,				/* ops */
	0,					/* ptr */
	0					/* next */
};

struct RamfsFile
{
	FileId id;
	unsigned int size;
	char* data;
	struct RamfsFile* next;
};

static struct RamfsFile* ramfs_files = 0;

int ramfs_allocate_file_for_node(struct FsNode* node)
{
	struct RamfsFile* file = 0;

	if(ramfs_files == 0)
	{
		ramfs_files = malloc(sizeof(struct RamfsFile));

		if(!ramfs_files)
			panic("cannot alloc first ram file");

		ramfs_files->id = 1;
		ramfs_files->size = 0;
		ramfs_files->data = 0;
		ramfs_files->next = 0;

		file->id = ramfs_files->id;

		return 1;
	}

	file = ramfs_files;

	while(file->next)
	{
		file = file->next;
	}

	file->next = malloc(sizeof(struct RamfsFile));

	if(!ramfs_files)
		panic("cannot alloc first ram file");

	file->next->id = file->id;
	file->next->size = 0;
	file->next->data = 0;
	file->next->next = 0;

	return 1;
}

int ramfs_add_child(struct FsNode* dir, struct FsNode* node)
{
	node->ops = &ramfs_ops;

	if(node->flags & FILE)
	{
		ramfs_allocate_file_for_node(node);
	}

	directory_add_node(dir, node);
}

struct FsNode* ramfs_get_root()
{
	return &ramfs_root;
}

