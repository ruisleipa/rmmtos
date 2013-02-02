#include "devfs.h"
#include "fs/directory.h"

static struct Node* devfs_root;

void devfs_init()
{
	devfs_root = directory_create_node("", STATIC, 0);
}

struct Node* devfs_get_root()
{
	return devfs_root;
}
