#ifndef FS_NODE_H
#define FS_NODE_H

#define FILE 0x0001
#define DIRECTORY 0x0002

/* The node is statically allocated and should not be freed when refcount drops to zero */
#define STATIC 0x8000
#define MOUNTPOINT 0x4000

#define HANDLE_READ 0x8000
#define HANDLE_WRITE 0x4000

#define ATTRIBUTE_MASK 0xff00
#define TYPE_MASK 0x00ff

struct Node
{
	char* name;
	unsigned int flags;

	unsigned int refcount;
	unsigned int writers;
	unsigned int readers;
};

struct Handle
{
	unsigned int flags;
	struct Node* node;
};

void node_init(struct Node* node, char* name, unsigned int flags);
void node_acquire(struct Node* node);
void node_release(struct Node* node);

#endif
