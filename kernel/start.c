#asm
#include "stack.s"
#endasm

#include "printf.h"
#include "int.h"
#include "task.h"
#include "malloc.h"

#include "clock.h"
#include "char/keyb.h"
#include "char/screen.h"
#include "block/floppy.h"
#include "panic.h"

#include "fs/node.h"
#include "fs/directory.h"
#include "fs/vfs.h"
#include "fs/ramfs/ramfs.h"
#include "fs/devfs/devfs.h"

#asm
	cli
	STACK_SET

	call _main
	import kernel_exit_force
	jmp word kernel_exit_force
#endasm

extern void* _end;

void tree(struct Node* root, int indent);
void print_cached_nodes(struct Directory* root, int indent);

void main() {
	struct Directory* root;
	struct Directory* dir;
	struct DirectoryHandle* handle;

	init_serial();
	screen_init();

	malloc_init((void*) &_end, (void*) 0xf000);

	intr_init();

	devfs_init();

	screen_init_device();
	clock_init();
	keyb_init();
	floppy_init();

	syscall_init();

	task_init();

	root = ramfs_create();
	vfs_set_root(root);

	handle = directory_open(root, HANDLE_WRITE);

	directory_add_node(handle, directory_create_node("devices", STATIC, 0));
	directory_add_node(handle, directory_create_node("floppy", STATIC, 0));

	directory_close(handle);

	dir = vfs_get_node_for_path("/devices");

	if (!dir) {
		panic("cannot find mountpoint for devfs");
	}

	directory_redirect(dir, devfs_get_root());

	printf("Filesystem (VFS):\n");
	tree(root, 0);
	printf("Filesystem (Debug):\n");
	print_cached_nodes(root, 0);

	malloc_stats();
	return;
}

void tree(struct Node* node, int indent) {
	int i, j;

	j = indent;

	while (j > 0)
		printf("\t", j--);

	printf("%x:'%s' %x\n", node, node->name, node->flags);

	if (node->flags & DIRECTORY) {
		struct DirectoryHandle* handle = directory_open(node, HANDLE_READ);
		struct Node* child = directory_get_next_node(handle, 0);

		while (child) {
			tree(child, indent + 1);

			child = directory_get_next_node(handle, child);
		}

		free(handle);
	}
}

void print_cached_nodes(struct Node* node, int indent) {
	int j;

	j = indent;

	while (j > 0)
		printf("\t", j--);

		printf("'%s' %x\n", node->name, node->flags);

	if (node->flags & DIRECTORY) {
		struct Directory* directory = node;
		struct Node* child;

		if (directory->redirect)
			directory = directory->redirect;

		child = directory->child;

		while (child) {
			print_cached_nodes(child, indent + 1);

			child = child->next;
		}
	}
}
