#ifndef FS_VFS_H
#define FS_VFS_H

#include "node.h"

#define RESOLUTION_OK 0
#define RESOLUTION_ISFILE 1
#define RESOLUTION_NOTFOUND 2
#define RESOLUTION_END 3

struct VfsPathResolution {
    char* path;
    char* end;
    char* current_part;

    struct Node* current_node;
};

void vfs_set_root(struct Node* node);

struct VfsPathResolution* vfs_start_resolve(char* path);
int vfs_advance_resolution(struct VfsPathResolution* resolution);

void vfs_free_path_resolution(struct VfsPathResolution* res);
struct Directory* vfs_create_filesystem(char* type, struct Node* node);

#endif

