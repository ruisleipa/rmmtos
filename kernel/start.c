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

#include "fs/node.h"

#asm
	cli
	STACK_SET

	call _main
	import kernel_exit_force
	jmp word kernel_exit_force
#endasm

extern void* _end;

void tree(struct Node* root, int indent);

void main()
{
	struct Node* root;

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

	vfs_set_root(devfs_get_root());

	/*tree(root, 0);
*/
	printf("%d\n", sizeof(unsigned long));

	malloc_stats();
	/*malloc_stats();
*/
	return;
}
/*
void tree(struct Node* root, int indent)
{
	int i, j;
	struct Node* child;

	for(i = 0; i < vfs_get_child_count(root); i++)
	{
		child = vfs_get_child_by_index(root, i);

		j = indent;

		while(j > 0)
			printf(" ", j--);

		printf("'%s' %x\n", child->name, child->flags);

		if(child->flags & DIRECTORY)
			tree(child, indent + 1);
	}
}
*/

