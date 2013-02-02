#include "directory.h"

#include "panic.h"

struct CachedNode
{
	struct Node* node;
	struct Directory* parent;
	struct CachedNode* next;
};

#define READ 1
#define WRITE 2

static char* non_dir = "dir op on non-dir";
static char* NOT_READ = "read op on non-read handle";
static char* NOT_WRITE = "write op on non-write handle";

static int add_cached_node(struct Directory* directory, struct Node* node)
{
	struct CachedNode* cached_node = malloc(sizeof(struct CachedNode));

	if(!cached_node)
		return 0;

	cached_node->node = node;
	cached_node->parent = directory;
	cached_node->next = directory->cached_nodes;
	directory->cached_nodes = cached_node;

	return 1;
}

struct Directory* directory_create_node(char* name, unsigned int flags, struct FileOps* ops)
{
	struct Directory* directory = malloc(sizeof(struct Directory));
	struct Node* node = directory;

	if(directory)
	{
		node_init(directory, name, DIRECTORY | (flags & ATTRIBUTE_MASK));

		directory->ops = ops;
		directory->cached_nodes = 0;
	}

	node_acquire(directory);

	return directory;
}

struct DirectoryHandle* directory_open(struct Node* node, unsigned int mode)
{
	struct DirectoryHandle* handle = 0;
	struct Directory* directory = 0;

	if(mode & HANDLE_WRITE && node->readers > 0)
		return 0;

	if(mode & HANDLE_READ && node->readers > 0)
		return 0;

	printf("mode: %x\n", mode);

	if(node->flags & DIRECTORY)
		directory = (struct Directory*)node;

	if(!directory)
		panic(non_dir);

	if(directory->ops && directory->ops->open)
		handle = directory->ops->open(directory, mode);
	else
		handle = malloc(sizeof(struct DirectoryHandle));

	if(handle)
	{
		handle->super.node = node;
		handle->super.flags = DIRECTORY | mode;

		if(mode & HANDLE_WRITE)
			handle->super.node->writers++;

		if(mode & HANDLE_READ)
			handle->super.node->readers++;

		node_acquire(node);
	}

	return handle;
}

void directory_close(struct DirectoryHandle* handle)
{
	if((handle->super.flags & ATTRIBUTE_MASK) & HANDLE_READ)
		handle->super.node->readers--;

	if((handle->super.flags & ATTRIBUTE_MASK) & HANDLE_WRITE)
		handle->super.node->writers--;

	node_release(handle->super.node);

	free(handle);
}

struct Node* directory_get_node(struct DirectoryHandle* handle)
{
	struct Directory* directory = handle->super.node;
	struct Node* node = 0;

	if(!((handle->super.flags & ATTRIBUTE_MASK) & HANDLE_READ))
		panic(NOT_READ);

	if(directory->ops && directory->ops->get_node)
	{
		node = directory->ops->get_node(handle);

		add_cached_node(directory, node);
	}

	if(node)
		node_acquire(node);

	return handle;
}

struct Node* directory_find_node(struct DirectoryHandle* handle, char* name)
{
	struct Directory* directory = handle->super.node;
	struct Node* node;
	struct CachedNode* cached_node;

	printf("handle->super.flags: %x\n", handle->super.flags);

	if(!((handle->super.flags & ATTRIBUTE_MASK) & HANDLE_READ))
		panic(NOT_READ);

	/* first check if the node is in memory */
	cached_node = directory->cached_nodes;

	while(cached_node)
	{
		if(strcmp(cached_node->node->name, name) == 0)
		{
			node = cached_node->node;
			break;
		}

		cached_node = cached_node->next;
	}

	/* not loaded, get it from the file system */
	if(!node && directory->ops && directory->ops->find_node)
	{
		node = directory->ops->find_node(handle, name);

		add_cached_node(directory, node);
	}

	if(node)
		node_acquire(node);

	return node;
}

struct Node* directory_add_node(struct DirectoryHandle* handle, struct Node* node)
{
	struct Directory* directory = handle->super.node;
	int result;

	if(!((handle->super.flags & ATTRIBUTE_MASK) & HANDLE_WRITE))
		panic(NOT_WRITE);

	if(directory->ops && directory->ops->add_node)
		result = directory->ops->add_node(handle, node);
	else
		result = 1;

	if(result)
	{
		add_cached_node(directory, node);
		node_acquire(node);
	}

	return node;
}
