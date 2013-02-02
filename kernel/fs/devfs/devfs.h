#ifndef FS_DEVFS_DEVFS_H
#define FS_DEVFS_DEVFS_H

#include "fs/node.h"

void devfs_init();
struct Node* devfs_get_root();

#endif

