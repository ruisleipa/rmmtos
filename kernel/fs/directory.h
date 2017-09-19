#ifndef FS_DIRECTORY_H
#define FS_DIRECTORY_H

#include "node.h"

struct DirectoryHandle
{
	struct Handle super;
};

struct DirectoryHandle* directory_open(struct Node* directory, unsigned int mode);
struct Node* directory_get_next_node(struct DirectoryHandle* handle, struct Node* current_node);
struct Node* directory_find_node(struct DirectoryHandle* handle, char* name);
struct Node* directory_add_node(struct DirectoryHandle* handle, struct Node* node);

void directory_redirect(struct Directory* directory, struct Directory* destination);

typedef struct DirectoryHandle* (DirectoryOpen)(struct Directory* directory);
typedef struct Node* (DirectoryGetNextNode)(struct DirectoryHandle* handle, struct Node* current_node);
typedef struct Node* (DirectoryFindNode)(struct DirectoryHandle* handle, char* name);
typedef int (DirectoryAddNode)(struct DirectoryHandle* handle, struct Node* node);

struct DirectoryOps
{
	DirectoryOpen* open;
	DirectoryGetNextNode* get_next_node;
	DirectoryFindNode* find_node;
	DirectoryAddNode* add_node;
};

struct Directory
{
	struct Node super;

	struct DirectoryOps* ops;
	struct Node* child;
        struct Directory* redirect;
};

#endif
