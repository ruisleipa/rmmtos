#include "file.h"
#include "panic.h"

static char* non_file = "file op on non-file";
static char* NOT_READ = "read op on non-read handle";
static char* NOT_WRITE = "write op on non-write handle";

struct File* file_create_node(char* name, unsigned int flags, struct FileOps* ops)
{
	struct File* file = malloc(sizeof(struct File));
	struct Node* node = file;

	debug_printf("file_create_node: \n", "", flags, ops);
	if(file)
	{
		node_init(file, name, FILE | (flags & ATTRIBUTE_MASK));

		file->ops = ops;
	}

	debug_printf("file_create_node: %s %x %x\n", node->name, node->flags, file->ops);

	node_acquire(node);

	return file;
}

struct FileHandle* file_open(struct Node* node, unsigned int mode)
{
	struct FileHandle* handle = 0;
	struct File* file = 0;

	if((mode & HANDLE_WRITE) && node->readers > 0)
		return 0;

	if((mode & HANDLE_READ) && node->writers > 0)
		return 0;

	if(node->flags & FILE)
		file = (struct File*)node;

	if(!file)
		panic(non_file);

	if(file->ops && file->ops->open)
		handle = file->ops->open(file, mode);
	else
		handle = malloc(sizeof(struct FileHandle));

	if(handle)
	{
		handle->super.node = node;
		handle->super.flags = FILE | mode;

		if(mode & HANDLE_WRITE)
			handle->super.node->writers++;

		if(mode & HANDLE_READ)
			handle->super.node->readers++;

		init64(&handle->position, 0, 0, 0, 0);

		node_acquire(node);
	}

	return handle;
}

void file_close(struct FileHandle* handle)
{
	if(handle->super.flags & ATTRIBUTE_MASK == HANDLE_READ)
		handle->super.node->readers--;

	if(handle->super.flags & ATTRIBUTE_MASK == HANDLE_WRITE)
		handle->super.node->writers--;

	node_release(handle->super.node);
	free(handle);
}

void file_seek(struct FileHandle* handle, Uint64* position)
{
	set64(&handle->position, position);
}

unsigned int file_read(struct FileHandle* handle, char* buffer, unsigned int size)
{
	struct File* file = handle->super.node;

	if(handle->super.flags & TYPE_MASK == FILE)
		panic(non_file);

	if(handle->super.flags & ATTRIBUTE_MASK == HANDLE_READ)
		panic(NOT_READ);

	if(file->ops && file->ops->read)
	{
		return file->ops->read(handle, buffer, size);
	}

	return 0;
}

unsigned int file_write(struct FileHandle* handle, char* buffer, unsigned int size)
{
	struct File* file = handle->super.node;

	if(handle->super.flags & TYPE_MASK == FILE)
		panic(non_file);

	if(handle->super.flags & ATTRIBUTE_MASK == HANDLE_WRITE)
		panic(NOT_WRITE);

	if(file->ops && file->ops->write)
		return file->ops->write(handle, buffer, size);

	return 0;
}
