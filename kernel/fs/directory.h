#ifndef FS_DIRECTORY_H
#define FS_DIRECTORY_H

#include "node.h"

struct CachedNode;

struct DirectoryHandle
{
	struct Handle super;
};

struct DirectoryHandle* directory_open(struct Node* directory, unsigned int mode);
struct Node* directory_get_node(struct DirectoryHandle* handle);
struct Node* directory_find_node(struct DirectoryHandle* handle, char* name);
struct Node* directory_add_node(struct DirectoryHandle* handle, struct Node* node);

typedef struct DirectoryHandle* (DirectoryOpen)(struct Directory* directory);
typedef struct Node* (DirectoryGetNode)(struct DirectoryHandle* handle);
typedef struct Node* (DirectoryFindNode)(struct DirectoryHandle* handle, char* name);
typedef int (DirectoryAddNode)(struct DirectoryHandle* handle, struct Node* node);

struct DirectoryOps
{
	DirectoryOpen* open;
	DirectoryGetNode* get_node;
	DirectoryFindNode* find_node;
	DirectoryAddNode* add_node;
};

struct Directory
{
	struct Node super;

	struct DirectoryOps* ops;
	struct CachedNode* cached_nodes;
};

#endif
