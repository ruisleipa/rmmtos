#include "node.h"

void node_init(struct Node* node, char* name, unsigned int flags)
{
	node->name = name;
	node->flags = flags;
	node->refcount = 0;
	node->writers = 0;
	node->readers = 0;
}

void node_acquire(struct Node* node)
{
	node->refcount++;
}

void node_release(struct Node* node)
{
	node->refcount--;
}
