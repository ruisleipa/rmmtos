#include "vfs.h"
#include "malloc.h"
#include "string.h"
#include "directory.h"
#include "devfs/devfs.h"
#include "ramfs/ramfs.h"
#include "fat/fat.h"
#include "panic.h"

char* ERROR_NODE_INV_OPS = "Node has invalid ops";

struct Node* vfs_root = 0;

struct VfsPathResolution* vfs_start_resolve(char* path)
{
	struct VfsPathResolution* resolution = 0;
	int path_size = strlen(path) + 1;
	char* path_copy;
	int i;

	resolution = malloc(sizeof(*resolution));

	if(!resolution) {
		return 0;
	}

	path_copy = malloc(path_size);

	strcpy(path_copy, path);

	if(!path_copy) {
		free(resolution);
		return 0;
	}

	resolution->path = path_copy;

	/* change directory separators to zero terminators */
	while(*path_copy != 0)
	{
		if(*path_copy == '/')
			*path_copy = 0;

		path_copy++;
	}

	resolution->end = resolution->path + path_size - 1;
	resolution->current_node = vfs_root;
	resolution->current_part = resolution->path;

	return resolution;
}

int vfs_advance_resolution(struct VfsPathResolution* resolution)
{
	debug_printf("current_part '%s'\n", resolution->current_part);

	while(*resolution->current_part != 0) {
		resolution->current_part++;
	}

	if(resolution->current_part == resolution->end)
		return RESOLUTION_END;

	resolution->current_part++;
	debug_printf("seeked_part '%s'\n", resolution->current_part);

	return vfs_retry_resolution(resolution);
}

int vfs_retry_resolution(struct VfsPathResolution* resolution)
{
	struct Node* node = resolution->current_node;

	if(strcmp(resolution->current_part, "") == 0)
		return RESOLUTION_OK;

	if(node->flags & DIRECTORY)
	{
		struct DirectoryHandle* handle = 0;

		handle = directory_open(node, HANDLE_READ);

		if(handle)
		{
			node = directory_find_node(handle, resolution->current_part);

			directory_close(handle);

			if(!node) {
				return RESOLUTION_NOTFOUND;
			}

			resolution->current_node = node;

			return RESOLUTION_OK;
		}
		else
		{
			return RESOLUTION_NOTFOUND;
		}
	}
	else
	{
		return RESOLUTION_ISFILE;
	}
}


void vfs_free_path_resolution(struct VfsPathResolution* resolution)
{
	free(resolution->path);
	free(resolution);
}

struct Node* vfs_get_node_for_path(char* path)
{
	struct VfsPathResolution* resolution = vfs_start_resolve(path);
	int status;
	struct Node* node = 0;

	do {
		status = vfs_advance_resolution(resolution);
	} while(status == RESOLUTION_OK);

	if (status == RESOLUTION_END)
		node = resolution->current_node;

	vfs_free_path_resolution(resolution);

	return node;
}

void vfs_set_root(struct Node* node)
{
	vfs_root = node;
}

struct Directory* vfs_create_filesystem(char* type, struct Node* node)
{
	if(strcmp(type, "fat") == 0) {
		return create_fat_fs(node);
	} else if(strcmp(type, "ram") == 0) {
		return ramfs_create();
	} else if(strcmp(type, "dev") == 0) {
		return devfs_get_root();
	}

	return 0;
}
