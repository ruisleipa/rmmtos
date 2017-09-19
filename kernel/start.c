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

void main()
{
	struct Directory* root;
	struct DirectoryHandle* handle;

	screen_init();

	malloc_init((void*)&_end,(void*)0xf000);

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

	directory_close(handle);

	directory_redirect(vfs_resolve_path("/devices"), devfs_get_root());

	tree(root, 0);
	print_cached_nodes(root, 0);

	malloc_stats();
	return;
}

void tree(struct Node* directory, int indent)
{
	int i, j;
	struct Node* child;
	struct DirectoryHandle* handle;

	handle = directory_open(directory, HANDLE_READ);

	child = directory_get_next_node(handle, 0);

	while(child) {
		j = indent;

		while(j > 0)
			printf(" ", j--);

		printf("'%s' %x\n", child->name, child->flags);

		if(child->flags & DIRECTORY)
			tree(child, indent + 1);

		child = directory_get_next_node(handle, child);
	}
}

void print_cached_nodes(struct Directory* directory, int indent)
{
	int i, j;
	struct Node* child;

	if(directory->redirect)
		directory = directory->redirect;

	child = directory->child;

	while(child) {
		j = indent;

		while(j > 0)
			printf(" ", j--);

		printf("'%s' %x\n", child->name, child->flags);

		if(child->flags & DIRECTORY)
			tree(child, indent + 1);

		child = child->next;
	}
}
