#include "vfs.h"
#include "malloc.h"
#include "string.h"

char* ERROR_NODE_INV_OPS = "Node has invalid ops";

static struct Node* vfs_root = 0;

struct Node* vfs_resolve_path(char* path)
{
	char* str = malloc(strlen(path) + 1);
	char* part;
	char* ptr;
	struct Node* node = vfs_root;
	struct DirectoryHandle* handle = 0;
	int first = 1;



	if(!str)
		return 0;

	strcpy(str, path);
	part = str;

	ptr = str;

	/* change directory separators to zero terminators */
	while(*ptr != 0)
	{
		if(*ptr == '/')
			*ptr = 0;

		ptr++;
	}

	while(*path != 0)
	{
		if(*path == '/')
		{
			path++;
			part++;

			if(first && (strcmp("", part) == 0))
			{
				/* root path, break out */
				break;
			}

			first = 0;

			if(node->flags & DIRECTORY)
			{
				handle = directory_open(node, HANDLE_READ);

				if(handle)
				{
					node = directory_find_node(handle, part);

					directory_close(handle);
				}
				else
				{
					node = 0;
				}
			}
			else
			{
				node = 0;
			}

			if(!node)
				break;
		}

		path++;
		part++;
	}

	free(str);

	return node;
}

void vfs_set_root(struct Node* node)
{
	vfs_root = node;
}
