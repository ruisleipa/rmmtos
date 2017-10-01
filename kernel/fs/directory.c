#include "directory.h"

#include "panic.h"

#define READ 1
#define WRITE 2

static char* non_dir = "dir op on non-dir";
static char* NOT_READ = "read op on non-read handle";
static char* NOT_WRITE = "write op on non-write handle";

void directory_add_to_children(struct Directory* directory, struct Node* node)
{
	debug_printf("adtc: %s %x %s %x \n", node->name, node->next, node->next->name, node->flags);

	if(node->next) {
		panic("adding node to two dirs");
	}

	// TODO: need to add somewhere else other than front?

	node->next = directory->child;
	directory->child = node;
}

struct Directory* directory_create_node(char* name, unsigned int flags, struct DirectoryOps* ops)
{
	struct Directory* directory = malloc(sizeof(struct Directory));

	if(directory)
	{
		node_init(directory, name, DIRECTORY | (flags & ATTRIBUTE_MASK));

		directory->ops = ops;
		directory->child = 0;
		directory->redirect = 0;

		node_acquire(directory);
	}

	return directory;
}

struct DirectoryHandle* directory_open(struct Node* node, unsigned int mode)
{
	struct DirectoryHandle* handle = 0;
	struct Directory* directory = 0;

	if(mode & HANDLE_WRITE && (node->readers > 0 || node->writers > 0))
		return 0;

	if(mode & HANDLE_READ && node->writers > 0)
		return 0;

	debug_printf("node: %s\n", node->name);

	if(node->flags & DIRECTORY)
		directory = (struct Directory*)node;

	if(!directory)
		panic(non_dir);

	if(directory->redirect)
		directory = directory->redirect;

	if(directory->ops && directory->ops->open)
		handle = directory->ops->open(directory, mode);
	else
		handle = malloc(sizeof(struct DirectoryHandle));

	if(handle)
	{
		handle->super.node = directory;
		handle->super.flags = DIRECTORY | mode;

		if(mode & HANDLE_WRITE)
			handle->super.node->writers++;

		if(mode & HANDLE_READ)
			handle->super.node->readers++;

		node_acquire(directory);
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

struct Node* directory_get_next_node(struct DirectoryHandle* handle, struct Node* current_node)
{
	struct Directory* directory = handle->super.node;
	struct Node* node = 0;

	if(!((handle->super.flags & ATTRIBUTE_MASK) & HANDLE_READ))
		panic(NOT_READ);

	if(directory->ops && directory->ops->get_next_node)
	{
		node = directory->ops->get_next_node(handle, current_node);

		if(node) {
			directory_add_to_children(directory, node);
		}
	}
	else
	{
		if(!current_node)
		{
			node = directory->child;
		}
		else
		{
			node = current_node->next;
		}
	}



	return node;
}

struct Node* directory_find_node(struct DirectoryHandle* handle, char* name)
{
	struct Directory* directory = handle->super.node;
	struct Node* node;

	debug_printf("handle->super.flags: %x\n", handle->super.flags);

	if(!((handle->super.flags & ATTRIBUTE_MASK) & HANDLE_READ))
		panic(NOT_READ);

	/* first check if the node is in memory */
	node = directory->child;

	while(node)
	{
		if(strcmp(node->name, name) == 0)
		{
			break;
		}

		node = node->next;
	}

	/* not loaded, get it from the file system */
	if(!node && directory->ops && directory->ops->find_node)
	{
		node = directory->ops->find_node(handle, name);
		
		directory_add_to_children(directory, node);
	}

	if(node) {
		node_acquire(node);
	}


	return node;
}

struct Node* directory_add_node(struct DirectoryHandle* handle, struct Node* node)
{
	struct Directory* directory = handle->super.node;
	int result;

	if(!((handle->super.flags & ATTRIBUTE_MASK) & HANDLE_WRITE))
		panic(NOT_WRITE);

	if(directory->ops && directory->ops->add_node)
	{
		node = directory->ops->add_node(handle, node);
	}
	else
	{
		directory_add_to_children(directory, node);
	}

	return node;
}

extern struct Node* vfs_root;

void directory_redirect(struct Node* node, struct Directory* destination)
{
	struct Directory* directory = 0;

	if(node->flags & DIRECTORY)
		directory = (struct Directory*)node;

	if(!directory)
		panic(non_dir);

	debug_printf("redirecting %s to %s\n", directory->super.name, destination->super.name);

	directory->redirect = destination;
}

