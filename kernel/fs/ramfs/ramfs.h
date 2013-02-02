#ifndef FS_RAMFS_RAMFS_H
#define FS_RAMFS_RAMFS_H

#include "fs/node.h"

int ramfs_allocate_file_for_node(struct FsNode* node);
void ramfs_directory_add_node(struct FsNode* dir, struct FsNode* node);
struct FsNode* ramfs_get_root();

#endif
