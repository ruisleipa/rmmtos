#ifndef FS_VFS_H
#define FS_VFS_H

#include "node.h"

void vfs_set_root(struct Node* node);
struct Node* vfs_resolve_path(char* path);

#endif

