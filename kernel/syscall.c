#include "syscall.h"
#include "task.h"
#include "panic.h"
#include "fs/vfs.h"
#include "fs/file.h"
#include "fs/directory.h"
#include "fs/node.h"
#include "uint64.h"

#define NUM_SYSCALLS 12

typedef unsigned int (syscall_t)(Task*);

extern struct Task* current_task;

static struct Uint64* get_int64(struct Task* task, unsigned int reg)
{
	unsigned int addr = task_get_register(task, reg);
	Uint64* n = malloc(sizeof(Uint64));

	if(!n)
		return 0;

	task_get_memory(task, addr, n, sizeof(Uint64));

	return n;
}

static char* get_string(struct Task* task, unsigned int reg)
{
	unsigned int addr = task_get_register(task, reg);
	char c;
	unsigned int length = 0;
	char* string;

	do
	{
		task_get_memory(task, addr, &c, 1);
		addr++;
		length++;
	}
	while(c != 0);

	string = malloc(length);

	if(!string)
		return 0;

	addr = task_get_register(task, reg);

	task_get_memory(task, addr, string, length);

	return string;
}

unsigned int sys_exit(struct Task* task)
{
	unsigned int code = task_get_register(task, BX);

	printf("sys_exit: %d\n", code);
	return -1;
}

unsigned int sys_open_file(struct Task* task)
{
	char* path = get_string(task, BX);
	unsigned int mode = task_get_register(task, CX);
	unsigned int internal_mode = 0;
	struct Node* node = 0;
	struct Handle* handle = 0;
	unsigned int id = 0;

	if(path)
		node = vfs_resolve_path(path);

	if(node)
	{
		if(mode & 1)
			internal_mode |= HANDLE_READ;
		if(mode & 2)
			internal_mode |= HANDLE_WRITE;

		handle = file_open(node, internal_mode);
	}

	if(handle)
		id = task_add_handle(task, handle);

	free(path);

	return id;
}

unsigned int sys_close_file(struct Task* task)
{
	int id = task_get_register(task, BX);
	struct Handle* handle = 0;

	handle = task_get_handle(task, id);

	if(handle)
	{
		file_close(handle);
		task_remove_handle(task, id);
	}

	return 0;
}

unsigned int sys_read_file(struct Task* task)
{
	int id = task_get_register(task, BX);
	int task_buffer = task_get_register(task, CX);
	unsigned int size = task_get_register(task, DX);
	struct Handle* handle = 0;
	char* buffer = 0;

	handle = task_get_handle(task, id);

	if(handle)
		buffer = malloc(size);

	if(buffer)
	{
		size = file_read(handle, buffer, size);

		task_set_memory(task, task_buffer, buffer, size);

		free(buffer);
	}
	else
	{
		size = 0;
	}

	return size;
}

unsigned int sys_write_file(struct Task* task)
{
	int id = task_get_register(task, BX);
	int task_buffer = task_get_register(task, CX);
	unsigned int size = task_get_register(task, DX);
	struct Handle* handle = 0;
	char* buffer = 0;

	handle = task_get_handle(task, id);

	if(handle)
		buffer = malloc(size);

	if(buffer)
	{
		task_get_memory(task, task_buffer, buffer, size);

		size = file_write(handle, buffer, size);

		free(buffer);
	}
	else
	{
		size = 0;
	}

	return size;
}

unsigned int sys_seek(struct Task* task)
{
	printf("sys_seek unimplemented\n");
	return -1;
}

unsigned int sys_remove(struct Task* task)
{
	printf("sys_remove unimplemented\n");
	return -1;
}

unsigned int sys_mount(struct Task* task)
{
	printf("sys_mount unimplemented\n");

	create_fat_fs(vfs_resolve_path("/floppy0"));

	return -1;
}

unsigned int sys_unmount(struct Task* task)
{
	printf("sys_unmount unimplemented\n");
	return -1;
}

unsigned int sys_fork(struct Task* task)
{
	fork();
	return 1;
}

unsigned int sys_exec(struct Task* task)
{
	char* path = get_string(task, BX);

	if(!path)
		return -1;

	printf("sys_exec: %s\n", path);

	free(path);

	return -1;
}

unsigned int sys_sleep(struct Task* task)
{
	clock_set_wakeup(task, task_get_register(task, BX));

	task_sleep(task);
}

syscall_t* syscall_table[NUM_SYSCALLS]=
{
	sys_exit,
	sys_open_file,
	sys_close_file,
	sys_read_file,
	sys_write_file,
	sys_seek,
	sys_remove,
	sys_mount,
	sys_unmount,
	sys_fork,
	sys_exec,
	sys_sleep
};

void syscall_func(unsigned int num)
{
	syscall_t* func;
	unsigned int i = task_get_register(current_task, AX);
	unsigned int result;

	if(i < NUM_SYSCALLS)
	{
		func = syscall_table[i];
		result = func(current_task);
	}
	else
	{
		result = -1;
	}

	task_set_register(current_task, AX, result);
}

void syscall_init()
{
	sys_capture(syscall_func);
}


